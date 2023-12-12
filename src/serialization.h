#ifndef __DG_CEREAL__
#define __DG_CEREAL__

#include <string>
#include <iostream>
#include <memory>
#include <vector>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <cstring>
#include <climits>
#include <optional>

namespace dg::compact_serializer::types{

    using hash_type     = uint64_t; 
    using size_type     = uint64_t;
}

namespace dg::compact_serializer::runtime_exception{

    struct corrupted_data: std::exception{}; 
}

namespace dg::compact_serializer::types_space{

    static constexpr auto nil_lambda    = [](...){}; 

    template <class T, class = void>
    struct is_tuple: std::false_type{};
    
    template <class T>
    struct is_tuple<T, std::void_t<decltype(std::tuple_size<T>::value)>>: std::true_type{};

    template <class T>
    struct is_unique_ptr: std::false_type{};

    template <class T>
    struct is_unique_ptr<std::unique_ptr<T>>: std::true_type{}; 

    template <class T>
    struct is_optional: std::false_type{};

    template <class ...Args>
    struct is_optional<std::optional<Args...>>: std::true_type{}; 

    template <class T>
    struct is_vector: std::false_type{};

    template <class ...Args>
    struct is_vector<std::vector<Args...>>: std::true_type{};

    template <class T>
    struct is_unordered_map: std::false_type{};

    template <class ...Args>
    struct is_unordered_map<std::unordered_map<Args...>>: std::true_type{}; 

    template <class T>
    struct is_unordered_set: std::false_type{};

    template <class ...Args>
    struct is_unordered_set<std::unordered_set<Args...>>: std::true_type{};

    template <class T>
    struct is_map: std::false_type{};

    template <class ...Args>
    struct is_map<std::map<Args...>>: std::true_type{}; 
    
    template <class T>
    struct is_set: std::false_type{};

    template <class ...Args>
    struct is_set<std::set<Args...>>: std::true_type{};

    template <class T, class = void>
    struct is_serializable: std::false_type{};

    template <class T>
    struct is_serializable<T, std::void_t<decltype(std::declval<T>().put(nil_lambda))>>: std::true_type{};
 
    template <class T>
    using base_type = std::remove_const_t<std::remove_reference_t<T>>;

    template <class T>
    struct recursive_const_strip: std::remove_const<T>{}; 

    template <class ...Args>
    struct recursive_const_strip<std::pair<Args...>>{
        using type = std::pair<typename recursive_const_strip<Args>::type...>;
    };

    template <class ...Args>
    struct recursive_const_strip<std::tuple<Args...>>{
        using type  = std::tuple<typename recursive_const_strip<Args>::type...>;
    };

    template <class T>
    using recursive_const_strip_t   = typename recursive_const_strip<T>::type;

    template <class ...Args>
    static constexpr auto is_basic_string_convertible_(std::basic_string<Args...>&) -> std::true_type;

    template <class T>
    static constexpr auto is_basic_string_convertible_(T&&) -> std::false_type; 

    template <class T>
    struct is_basic_string_convertile: decltype(is_basic_string_convertible_(std::declval<T&>())){}; 

    template <class T>
    static constexpr bool is_container_v    = std::disjunction_v<is_vector<T>, is_unordered_map<T>, is_unordered_set<T>, is_map<T>, is_set<T>, is_basic_string_convertile<T>>;

    template <class T>
    static constexpr bool is_tuple_v        = is_tuple<T>::value; 

    template <class T>
    static constexpr bool is_unique_ptr_v   = is_unique_ptr<T>::value;

    template <class T>
    static constexpr bool is_optional_v     = is_optional<T>::value;

    template <class T>
    static constexpr bool is_nillable_v     = is_unique_ptr_v<T> | is_optional_v<T>; 

    template <class T>
    static constexpr bool is_serializable_v = is_serializable<T>::value;

}

namespace dg::compact_serializer::utility{

    using namespace compact_serializer::types;

    struct SyncedEndiannessService{
        
        static constexpr auto is_native_big      = bool{std::endian::native == std::endian::big};
        static constexpr auto is_native_little   = bool{std::endian::native == std::endian::little};
        static constexpr auto precond            = bool{(is_native_big ^ is_native_little) != 0};
        static constexpr auto deflt              = std::endian::little; 
        static constexpr auto native_uint8       = is_native_big ? uint8_t{0} : uint8_t{1}; 

        static_assert(precond); //xor

        template <class T, std::enable_if_t<std::is_arithmetic_v<T>, bool> = true>
        static constexpr T bswap(T value){
            
            constexpr auto LOWER_BIT_MASK   = ~((char) 0u);
            constexpr auto idx_seq          = std::make_index_sequence<sizeof(T)>();
            T rs{};

            [&]<size_t ...IDX>(const std::index_sequence<IDX...>&){
                (
                    [&](size_t){

                        rs <<= CHAR_BIT;
                        rs |= value & LOWER_BIT_MASK;
                        value >>= CHAR_BIT;

                    }(IDX), ...
                );
            }(idx_seq);

            return rs;
        }

        template <class T, std::enable_if_t<std::is_arithmetic_v<T>, bool> = true>
        static inline void dump(void * dst, T data) noexcept{    

            if constexpr(std::endian::native != deflt){
                data = bswap(data);
            }

            std::memcpy(dst, &data, sizeof(T));
        }

        template <class T, std::enable_if_t<std::is_arithmetic_v<T>, bool> = true>
        static inline T load(const void * src) noexcept{
            
            T rs{};
            std::memcpy(&rs, src, sizeof(T));

            if constexpr(std::endian::native != deflt){
                rs = bswap(rs);
            }

            return rs;
        }

        static inline const auto bswap_lambda   = []<class ...Args>(Args&& ...args){return bswap(std::forward<Args>(args)...);}; 

    };

    auto hash(const char * buf, size_t sz) noexcept -> hash_type{ //3rd world implementation
    
        using _MemIO        = SyncedEndiannessService;

        const char * ibuf   = buf; 
        const size_t CYCLES = sz / sizeof(hash_type);
        const hash_type MOD = std::numeric_limits<hash_type>::max() >> 1;
        hash_type total     = {};
        hash_type cur       = {};

        for (size_t i = 0; i < CYCLES; ++i){
            cur     = _MemIO::load<hash_type>(ibuf);
            cur     %= MOD;
            total   += cur;
            total   %= MOD;
            ibuf    += sizeof(hash_type); 
        }

        return total;
    }

    template <class T, std::enable_if_t<std::disjunction_v<types_space::is_vector<T>, 
                                                           types_space::is_basic_string_convertile<T>>, bool> = true>
    constexpr auto get_inserter(){

        auto inserter   = []<class ...Args>(T& container, Args&& ...args){
            container.push_back(std::forward<Args>(args)...);
        };

        return inserter;
    }

    template <class T, std::enable_if_t<std::disjunction_v<types_space::is_unordered_map<T>, types_space::is_unordered_set<T>, 
                                                           types_space::is_map<T>, types_space::is_set<T>>, bool> = true>
    constexpr auto get_inserter(){ 

        auto inserter   = []<class ...Args>(T& container, Args&& ...args){
            container.insert(std::forward<Args>(args)...);
        };

        return inserter;
    }

    template <class LHS, class ...Args, std::enable_if_t<types_space::is_unique_ptr_v<types_space::base_type<LHS>>, bool> = true>
    void initialize(LHS&& lhs, Args&& ...args){

        using pointee_type = std::remove_reference_t<decltype(*lhs)>;
        lhs = std::make_unique<pointee_type>(std::forward<Args>(args)...);
    }

    template <class LHS, class ...Args, std::enable_if_t<types_space::is_optional_v<types_space::base_type<LHS>>, bool> = true>
    void initialize(LHS&& lhs, Args&& ...args){

        using pointee_type  = std::remove_reference_t<decltype(*lhs)>;
        lhs = {std::forward<Args>(args)...};
    }

}

namespace dg::compact_serializer::archive{

    template <class BaseArchive>
    struct ForwardSerialization{
        
        using Self = ForwardSerialization;
        BaseArchive _base_archive;

        ForwardSerialization(BaseArchive _base_archive): _base_archive(_base_archive){} 

        template <class T, std::enable_if_t<std::is_arithmetic_v<types_space::base_type<T>>, bool> = true>
        void put(char *& buf, T&& data) const noexcept{
            
            static_assert(noexcept(this->_base_archive(buf, std::forward<T>(data))));
            this->_base_archive(buf, std::forward<T>(data));
        }

        template <class T, std::enable_if_t<types_space::is_nillable_v<types_space::base_type<T>>, bool> = true>
        void put(char *& buf, T&& data) const noexcept{

            bool has_data   = bool{data};
            put(buf, static_cast<char>(has_data));

            if (has_data){
                put(buf, *data);
            }
        }

        template <class T, std::enable_if_t<types_space::is_tuple_v<types_space::base_type<T>>, bool> = true>
        void put(char *& buf, T&& data) const noexcept{

            using btype         = types_space::base_type<T>;
            const auto idx_seq  = std::make_index_sequence<std::tuple_size_v<btype>>{};

            []<size_t ...IDX>(const Self& _self, char *& buf, T&& data, const std::index_sequence<IDX...>){
                (_self.put(buf, std::get<IDX>(data)), ...);
            }(*this, buf, std::forward<T>(data), idx_seq);
        }

        template <class T, std::enable_if_t<types_space::is_container_v<types_space::base_type<T>>, bool> = true>
        void put(char *& buf, T&& data) const noexcept{
            
            put(buf, static_cast<types::size_type>(data.size())); 

            for (const auto& e: data){
                put(buf, e);
            }
        }

        template <class T, std::enable_if_t<types_space::is_serializable_v<types_space::base_type<T>>, bool> = true>
        void put(char *& buf, T&& data) const noexcept{

            auto _self      = Self(this->_base_archive);
            auto archiver   = [=, &buf]<class ...Args>(Args&& ...args){
                (_self.put(buf, std::forward<Args>(args)), ...);
            };

            data.put(archiver);
        }
    };

    struct BackwardSerialization{

        using Self  = BackwardSerialization;

        template <class T, std::enable_if_t<std::is_arithmetic_v<types_space::base_type<T>>, bool> = true>
        void put(const char *& buf, T&& data) const{

            using btype     = types_space::base_type<T>;
            using _MemIO    = utility::SyncedEndiannessService;
            data            = _MemIO::load<btype>(buf);
            buf             += sizeof(btype);
        }

        template <class T, std::enable_if_t<types_space::is_nillable_v<types_space::base_type<T>>, bool> = true>
        void put(const char *& buf, T&& data) const{

            using obj_type  = std::remove_reference_t<decltype(*data)>;
            char status     = {}; 
            put(buf, status);

            if (static_cast<bool>(status)){
                auto obj    = obj_type{};
                put(buf, obj);
                utility::initialize(std::forward<T>(data), std::move(obj));
            } else{
                data    = {};
            }
        }

        template <class T, std::enable_if_t<types_space::is_tuple_v<types_space::base_type<T>>, bool> = true>
        void put(const char *& buf, T&& data) const{

            using btype         = types_space::base_type<T>;
            const auto idx_seq  = std::make_index_sequence<std::tuple_size_v<btype>>{};

            []<size_t ...IDX>(const Self& _self, const char *& buf, T&& data, const std::index_sequence<IDX...>){
                (_self.put(buf, std::get<IDX>(data)), ...);
            }(*this, buf, std::forward<T>(data), idx_seq);
        }

        template <class T, std::enable_if_t<types_space::is_container_v<types_space::base_type<T>>, bool> = true>
        void put(const char *& buf, T&& data) const{
            
            using btype     = typename types_space::base_type<T>;
            using elem_type = types_space::recursive_const_strip_t<typename btype::value_type>;
            auto sz         = types::size_type{}; 
            auto _ins       = utility::get_inserter<types_space::base_type<T>>();

            put(buf, sz); 
            data.reserve(sz);

            for (size_t i = 0; i < sz; ++i){
                elem_type i_e{};
                put(buf, i_e);    
                _ins(data, std::move(i_e));
            }
        }

        template <class T, std::enable_if_t<types_space::is_serializable_v<types_space::base_type<T>>, bool> = true>
        void put(const char *& buf, T&& data) const{

            auto archiver   = [&buf]<class ...Args>(Args&& ...args){ //
                (Self().put(buf, std::forward<Args>(args)), ...);
            };

            data.put(archiver);
        }
    };

}

namespace dg::compact_serializer::core{

    template <class T>
    auto count(const T& obj) noexcept -> size_t{

        char * buf          = nullptr;
        size_t bcount       = 0u;
        auto counter_lambda = [&]<class U>(char *&, U&& val) noexcept{
            bcount += sizeof(types_space::base_type<U>);
        };
        archive::ForwardSerialization _seri_obj(counter_lambda);
        _seri_obj.put(buf, obj);

        return bcount;            
    }

    template <class T>
    void serialize(const T& obj, char * buf) noexcept{

        auto base_lambda    = []<class U>(char *& buf, U&& val) noexcept{
            using base_type = types_space::base_type<U>;
            using _MemUlt   = utility::SyncedEndiannessService;
            _MemUlt::dump(buf, std::forward<U>(val));
            buf += sizeof(base_type);
        };

        archive::ForwardSerialization _seri_obj(base_lambda);
        _seri_obj.put(buf, obj);
    } 

    template <class T>
    void deserialize(const char * buf, T& obj){

        archive::BackwardSerialization().put(buf, obj);
    }

    constexpr auto integrity_header_size() -> size_t{

        return sizeof(types::hash_type) + sizeof(types::size_type);
    } 

    template <class T>
    auto integrity_count(const T& obj) noexcept -> size_t{

        return integrity_header_size() + count(obj);
    }

    template <class T>
    void integrity_serialize(const T& obj, char * buf) noexcept{

        using _MemIO    = utility::SyncedEndiannessService;
        auto sz         = static_cast<types::size_type>(count(obj));
        auto bbuf       = buf + integrity_header_size(); 

        serialize(obj, bbuf);
        _MemIO::dump(buf, utility::hash(bbuf, sz));
        _MemIO::dump(buf + sizeof(types::hash_type), sz);
    }

    template <class T>
    void integrity_deserialize(const char * buf, size_t sz, T& obj){

        if (sz < integrity_header_size()){
            throw runtime_exception::corrupted_data{};
        }

        using _MemIO    = utility::SyncedEndiannessService;
        auto hash_val   = _MemIO::load<types::hash_type>(buf);
        auto ssz        = _MemIO::load<types::size_type>(buf + sizeof(types::hash_type));
        auto total_sz   = ssz + integrity_header_size();

        if (total_sz != sz){
            throw runtime_exception::corrupted_data{};
        }

        if (utility::hash(buf + integrity_header_size(), ssz) != hash_val){
            throw runtime_exception::corrupted_data{};
        }

        deserialize(buf + integrity_header_size(), obj);
    }

}

namespace dg::compact_serializer{

    template <class T>
    auto serialize(const T& obj) -> std::pair<std::unique_ptr<char[]>, size_t>{

        auto bcount = core::integrity_count(obj);
        auto buf    = std::unique_ptr<char[]>(new char[bcount]);
        core::integrity_serialize(obj, buf.get());

        return {std::move(buf), bcount};
    } 

    template <class T>
    auto deserialize(const char * buf, size_t sz) -> T{

        T rs{};
        core::integrity_deserialize(buf, sz, rs);

        return rs;
    }
}

#endif