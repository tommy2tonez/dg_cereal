#ifndef __DG_CEREAL__
#define __DG_CEREAL__

#include <string>
#include <memory>
#include <vector>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <cstring>
#include <climits>
#include <bit>
#include <optional>
#include <numeric>

namespace dg::compact_serializer::constants{

    static constexpr auto endianness    = std::endian::little;
}

namespace dg::compact_serializer::types{

    using hash_type     = uint64_t; 
    using size_type     = uint64_t;
}

namespace dg::compact_serializer::runtime_exception{

    struct CorruptedError: std::exception{}; 
}

namespace dg::compact_serializer::types_space{

    static constexpr auto nil_lambda    = [](...){}; 

    template <class T, class = void>
    struct is_tuple: std::false_type{};
    
    template <class T>
    struct is_tuple<T, std::void_t<decltype(std::tuple_size<T>::value)>>: std::true_type{};

    template <class T, class = void>
    struct is_unique_ptr: std::false_type{};

    template <class T>
    struct is_unique_ptr<std::unique_ptr<T>, std::void_t<std::enable_if_t<!std::is_array_v<T>>>>: std::true_type{}; 

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

    template <class T>
    struct is_basic_string: std::false_type{};

    template <class ...Args>
    struct is_basic_string<std::basic_string<Args...>>: std::true_type{};

    template <class T, class = void>
    struct is_reflectible: std::false_type{};

    template <class T>
    struct is_reflectible<T, std::void_t<decltype(std::declval<T>().dg_reflect(nil_lambda))>>: std::true_type{};
    
    template <class T, class U, class = void>
    struct has_same_size: std::false_type{};

    template <class T, class U>
    struct has_same_size<T, U, std::void_t<std::enable_if_t<sizeof(T) == sizeof(U), bool>>>: std::true_type{};

    template <class T, class = void>
    struct is_dg_arithmetic: std::is_arithmetic<T>{};

    template <class T>
    struct is_dg_arithmetic<T, std::void_t<std::enable_if_t<std::is_floating_point_v<T>>>>: std::bool_constant<std::numeric_limits<T>::is_iec559>{}; 

    template <class T, class = void>
    struct containee_type{};

    template <class T>
    struct containee_type<T, std::void_t<std::enable_if_t<std::disjunction_v<is_vector<T>, is_unordered_set<T>, is_set<T>, is_basic_string<T>>>>>{
        using type  = typename T::value_type;
    };

    template <class T>
    struct containee_type<T, std::void_t<std::enable_if_t<std::disjunction_v<is_unordered_map<T>, is_map<T>>>>>{
        using type  = std::pair<typename T::key_type, typename T::mapped_type>;
    };

    template <class T>
    static constexpr bool is_container_v    = std::disjunction_v<is_vector<T>, is_unordered_map<T>, is_unordered_set<T>, is_map<T>, is_set<T>, is_basic_string<T>>;

    template <class T>
    static constexpr bool is_tuple_v        = is_tuple<T>::value; 

    template <class T>
    static constexpr bool is_unique_ptr_v   = is_unique_ptr<T>::value;

    template <class T>
    static constexpr bool is_optional_v     = is_optional<T>::value;

    template <class T>
    static constexpr bool is_nillable_v     = is_unique_ptr_v<T> | is_optional_v<T>; 

    template <class T>
    static constexpr bool is_reflectible_v  = is_reflectible<T>::value;

    template <class T>
    using base_type                         = std::remove_const_t<std::remove_reference_t<T>>;

    template <class T>
    static constexpr bool is_dg_arithmetic_v    = is_dg_arithmetic<T>::value;
}

namespace dg::compact_serializer::utility{

    using namespace compact_serializer::types;

    struct SyncedEndiannessService{
        
        static constexpr auto is_native_big      = bool{std::endian::native == std::endian::big};
        static constexpr auto is_native_little   = bool{std::endian::native == std::endian::little};
        static constexpr auto precond            = bool{(is_native_big ^ is_native_little) != 0};
        static constexpr auto deflt              = constants::endianness; 
        static constexpr auto native_uint8       = is_native_big ? uint8_t{0} : uint8_t{1}; 

        static_assert(precond); //xor

        template <class T, std::enable_if_t<std::is_arithmetic_v<T>, bool> = true>
        static inline T bswap(T value){
            
            char src[sizeof(T)]; 
            char dst[sizeof(T)];
            const auto idx_seq  = std::make_index_sequence<sizeof(T)>();
            
            std::memcpy(src, &value, sizeof(T));
            [&]<size_t ...IDX>(const std::index_sequence<IDX...>){
                ((dst[IDX] = src[sizeof(T) - IDX - 1]), ...);
            }(idx_seq);
            std::memcpy(&value, dst, sizeof(T));

            return value;
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

    //not working for double/ float 
    template <class T, class U, std::enable_if_t<std::conjunction_v<std::has_unique_object_representations<T>, 
                                                                    std::has_unique_object_representations<U>, 
                                                                    std::is_trivial<T>, 
                                                                    std::is_trivial<U>,
                                                                    types_space::has_same_size<T, U>>, bool> = true>
    auto bit_cast(const U& caster) -> T{

        auto rs     = T{};
        auto src    = static_cast<const void *>(&caster);
        auto dst    = static_cast<void *>(&rs);

        std::memcpy(dst, src, sizeof(T));

        return rs;
    };

    auto checksum(const char * buf, size_t sz) noexcept -> hash_type{

        auto accumulator    = [](hash_type cur, char nnext){return cur + bit_cast<uint8_t>(nnext);};
        return std::accumulate(buf, buf + sz, hash_type{0u}, accumulator);
    } 

    auto hash(const char * buf, size_t sz) noexcept -> hash_type{ //2nd world implementation
    
        using _MemIO        = SyncedEndiannessService;

        const char * ibuf   = buf; 
        const char * ebuf   = buf + sz;
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

        auto rem_sz         = static_cast<size_t>(std::distance(ibuf, ebuf));
        auto rem            = checksum(ibuf, rem_sz) % MOD;
        
        return total + rem;
    }

    template <class T, std::enable_if_t<std::disjunction_v<types_space::is_vector<T>, 
                                                           types_space::is_basic_string<T>>, bool> = true>
    constexpr auto get_inserter(){

        auto inserter   = []<class U, class ...Args>(U&& container, Args&& ...args){
            container.push_back(std::forward<Args>(args)...);
        };

        return inserter;
    }

    template <class T, std::enable_if_t<std::disjunction_v<types_space::is_unordered_map<T>, types_space::is_unordered_set<T>, 
                                                           types_space::is_map<T>, types_space::is_set<T>>, bool> = true>
    constexpr auto get_inserter(){ 

        auto inserter   = []<class U, class ...Args>(U&& container, Args&& ...args){
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
    struct Forward{
        
        using Self = Forward;
        BaseArchive base_archive;

        Forward(BaseArchive base_archive): base_archive(base_archive){} 

        template <class T, std::enable_if_t<types_space::is_dg_arithmetic_v<types_space::base_type<T>>, bool> = true>
        void put(char *& buf, T&& data) const noexcept{
            
            static_assert(noexcept(this->base_archive(buf, std::forward<T>(data))));
            this->base_archive(buf, std::forward<T>(data));
        }

        template <class T, std::enable_if_t<types_space::is_nillable_v<types_space::base_type<T>>, bool> = true>
        void put(char *& buf, T&& data) const noexcept{

            put(buf, bool{data});

            if (bool{data}){
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

        template <class T, std::enable_if_t<types_space::is_reflectible_v<types_space::base_type<T>>, bool> = true>
        void put(char *& buf, T&& data) const noexcept{

            auto _self      = Self(this->base_archive);
            auto archiver   = [=, &buf]<class ...Args>(Args&& ...args){ //REVIEW: rm [&]
                (_self.put(buf, std::forward<Args>(args)), ...);
            };

            data.dg_reflect(archiver);
        }
    };

    struct Backward{

        using Self  = Backward;

        template <class T, std::enable_if_t<types_space::is_dg_arithmetic_v<types_space::base_type<T>>, bool> = true>
        void put(const char *& buf, T&& data) const{

            using btype     = types_space::base_type<T>;
            using _MemIO    = utility::SyncedEndiannessService;
            data            = _MemIO::load<btype>(buf);
            buf             += sizeof(btype);
        }

        template <class T, std::enable_if_t<types_space::is_nillable_v<types_space::base_type<T>>, bool> = true>
        void put(const char *& buf, T&& data) const{

            using obj_type  = std::remove_reference_t<decltype(*data)>;
            bool status     = {}; 
            put(buf, status);

            if (status){
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
            
            using btype     = types_space::base_type<T>;
            using elem_type = types_space::containee_type<btype>::type;
            auto sz         = types::size_type{}; 
            auto isrter     = utility::get_inserter<btype>();

            put(buf, sz); 
            data.reserve(sz);

            for (size_t i = 0; i < sz; ++i){
                elem_type e{};
                put(buf, e);    
                isrter(data, std::move(e));
            }
        }

        template <class T, std::enable_if_t<types_space::is_reflectible_v<types_space::base_type<T>>, bool> = true>
        void put(const char *& buf, T&& data) const{

            auto archiver   = [&buf]<class ...Args>(Args&& ...args){ //REVIEW: rm [&]
                (Self().put(buf, std::forward<Args>(args)), ...);
            };

            data.dg_reflect(archiver);
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
        archive::Forward _seri_obj(counter_lambda);
        _seri_obj.put(buf, obj);

        return bcount;            
    }

    template <class T>
    auto serialize(const T& obj, char * buf) noexcept -> char *{

        auto base_lambda    = []<class U>(char *& buf, U&& val) noexcept{
            using base_type = types_space::base_type<U>;
            using _MemUlt   = utility::SyncedEndiannessService;
            _MemUlt::dump(buf, std::forward<U>(val));
            buf += sizeof(base_type);
        };

        archive::Forward _seri_obj(base_lambda);
        _seri_obj.put(buf, obj);

        return buf;
    } 

    template <class T>
    auto deserialize(const char * buf, T& obj) -> const char *{

        archive::Backward().put(buf, obj);
        return buf;
    }

    template <class T>
    auto integrity_count(const T& obj) noexcept -> size_t{

        return static_cast<size_t>(sizeof(types::hash_type)) + count(obj);
    }

    template <class T>
    auto integrity_serialize(const T& obj, char * buf) noexcept -> char *{

        using _MemIO    = utility::SyncedEndiannessService;
        auto bbuf       = buf + sizeof(types::hash_type); 
        auto ebuf       = serialize(obj, bbuf);
        auto sz         = static_cast<size_t>(std::distance(bbuf, ebuf)); 
        auto hashed     = utility::hash(bbuf, sz);

        _MemIO::dump(buf, hashed);

        return ebuf;
    }

    template <class T>
    auto integrity_deserialize(const char * buf, size_t sz, T& obj) -> const char * {

        if (sz < sizeof(types::hash_type)){
            throw runtime_exception::CorruptedError{};
        }

        using _MemIO    = utility::SyncedEndiannessService;
        auto hash_val   = _MemIO::load<types::hash_type>(buf);
        auto data       = buf + sizeof(types::hash_type);
        auto data_sz    = sz - sizeof(types::hash_type);

        if (utility::hash(data, data_sz) != hash_val){
            throw runtime_exception::CorruptedError{};
        }

        return deserialize(data, obj);
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