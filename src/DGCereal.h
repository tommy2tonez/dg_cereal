#ifndef __DGCEREAL__
#define __DGCEREAL__

#include <type_traits>
#include <memory>
#include <unordered_map>
#include <vector>
#include <unordered_set>
#include <set>
#include <map>
#include <deque>
#include <stack>
#include <queue>

namespace dgstd{

    template <class T>
    struct remove_reference{

        using type = T;

    };

    template <class T>
    struct remove_reference<T&>{

        using type = T;

    };

    template <class T>
    struct remove_reference<T&&>{

        using type = T;
        
    };

    template <class T>
    struct remove_reference<std::reference_wrapper<T>>{
        
        using type = typename remove_reference<T>::type;

    };

    template <class T>
    struct remove_reference<std::reference_wrapper<T>&>{

        using type = typename remove_reference<T>::type;

    };

    template <class T>
    struct remove_reference<std::reference_wrapper<T>&&>{

        using type = typename remove_reference<T>::type;

    };

};

namespace dg::serializer{

    template <class T>
    class ByteSequenceCountable{

        public:

            size_t size(){

                return static_cast<T *>(this)->size();

            }

    };

    template <class T>
    class CategorizedByteSequenceCountable: public ByteSequenceCountable<T>{

        public:

            template <unsigned BYTE_LENGTH>
            size_t categorized_size(){

                return static_cast<T *>(this)->template categorized_size<BYTE_LENGTH>();

            } 

    };

    template <class T>
    class CategorizedByteSequenceTickable: public CategorizedByteSequenceCountable<T>{

        public:

            template <unsigned BYTE_LENGTH>
            void tick(){

                static_cast<T *>(this)->template tick_template<BYTE_LENGTH>();

            }

            template <unsigned BYTE_LENGTH>
            void tick_many(size_t multiplier){

                static_cast<T *>(this)->template tick_many<BYTE_LENGTH>(multiplier);

            }

    };

    template <class T>
    class Archivable{

        public:

            template <class T, class ...Args>
            void put(Archivable<T>& arch, Args&& ...args){

                static_cast<T *>(this)->put(arch, std::forward<Args>(args)...);

            } 

            template <class T1>
            constexpr bool is_archivable() const{

                return T().template is_archivable<T1>();

            }

            Archivable<T> * to_archivable(){

                return this;

            }
            
    };

    template <class T>
    class Serializable{

        public: 

            template <class T1>
            void serialize(Archivable<T1>& archive){

                static_cast<T *>(this)->serialize(archive);

            }   

    };

    template <class T>
    class LinearPODReadable{

        public:

            auto& get(size_t idx){

                return static_cast<T *>(this)->get(idx); 

            }

    };

    template <class T>
    class LinearPODOperatable: public LinearPODReadable<T>{

        public:

            template <class T1>
            void set(size_t idx, T1&& val){

                static_cast<T *>(this)->set(idx, std::forward<T1>(val));

            }
    };

    template <class T>
    class ByteSequence: public ByteSequenceCountable<T>{ 

        public:

            char& get(size_t idx){

                return static_cast<T *>(this)->get(idx);

            }

    };

    template <class T>
    class ByteStreamable: public ByteSequence<T>{

        public:

            template <class T1, std::enable_if_t<std::is_pod<T1>::value>, bool> = true>
            void add(T1 val){

                static_cast<T *>(this)->add(val);

            }

    };

    template <class T>
    class LinearStreamable: public ByteStreamable<T>{

        public:

            template <class T1, std::enable_if_t<std::id_pod<T1>::value>, bool> = true>
            void add_linear(T1 * val, size_t length){

                static_cast<T *>(this)->add_linear(val, length);

            }

    };

    template <class T>
    class ByteSequenceIterable: public ByteSequenceCountable<T>{

        public:

            auto next(size_t sz){

                return static_cast<T *>(this)->next(sz);

            }

            template <unsigned BYTE_LENGTH>
            auto next_(){

                return static_cast<T *>(this)->template next_<BYTE_LENGTH>();

            }
            

    };

    template <class T>
    class DataInitializable{

        public:

            template <class T1, std::enable_if_t<std::is_pod<T1>::value, bool> = true>
            void init(T1& dst, T1 data){

                static_cast<T *>(this)->init(dst, data);

            };

    };
    
    template <class T, class ...Args>
    struct PtrTypeGetter{

        using type = PtrTypeGetter<T, Args...>::type *;  

    };

    template <class T>
    struct PtrTypeGetter{

        using type = T;

    };

    template <class T>
    class Transformable{

        public:

            template <class T1>
            static auto transform(T1&& val){

                return T::transform(std::forward<T1>(val)); 

            }

            template <class T1>
            static auto inverse(T1&& val){

                return T::inverse(std::forward<T1>(val));
                
            }

    };

    class DefaultTransformFunctor : public Transformable<DefaultTransformFunctor>{

        public:

            template <class T1>
            static auto transform(T1&& val){

                if constexpr(dgstd::is_reference<T1>::value){

                    return std::ref(val);

                } else{

                    return val; 

                }

            }

            template <class T1>
            static auto inverse(T1&& val){

                if constexpr(dgstd::is_reference<T1>::value){

                    return std::ref(val);

                } else{

                    return val; 

                }

            }

    };

    template <class T, class SZ_TYPE, class _Allocator_Policy>
    struct StableLinearDataWrapper{

        T *& ptr;
        SZ_TYPE& n; 

        StableLinearDataWrapper(T *& ptr, SZ_TYPE& n): ptr(ptr), n(n){};

    };

    template <class T, class _Allocator_Policy>
    struct StablePtrWrapper{

        T *& ptr;

        StablePtrWrapper(T *& ptr): ptr(ptr){};

    };

    template <class ...Args>
    class PolyRegisteredClasses{};

    template <class VIRTUAL_TYPE, class TRUE_TYPE, class _Allocator_Policy, bool IS_DYNAMIC_CAST = true>
    struct StablePolymorphicPtrWrapper{

        VIRTUAL_TYPE *& ptr;

        StablePolymorphicPtrWrapper(T *& ptr): ptr(ptr){};

    };

    template <class T, class RegisteredClasses, class _Allocator_Policy>
    struct StableAutoPolymorphicPtrWrapper{

        T *& ptr;

        StableAutoPolymorphicPtrWrapper(T *& ptr): ptr(ptr){};

    };

    template <class T>
    struct SharedPtrWrapper{

        std::shared_ptr<T>& ptr;

        SharedPtrWrapper(std::shared_ptr<T>& ptr): ptr(ptr){};

    };

    template <class VIRTUAL_TYPE, class TRUE_TYPE, bool IS_DYNAMIC_CAST = true>
    struct PolymorphicSharedPtrWrapper{

        std::shared_ptr<VIRTUAL_TYPE>& ptr;

        PolymorphicSharedPtrWrapper(std::shared_ptr<VIRTUAL_TYPE>& ptr): ptr(ptr){};

    };

    template <class T, class RegisteredClasses>
    struct AutoPolymorphicSharedPtrWrapper{

        std::shared_ptr<T>& ptr;

        AutoPolymorphicSharedPtrWrapper(std::shared_ptr<T>& ptr): ptr(ptr){};

    };

    template <class T>
    struct UniquePtrWrapper{

        std::unique_ptr<T>& ptr;

        UniquePtrWrapper(std::unique_ptr<T>& ptr): ptr(ptr){};

    };

    template <class VIRTUAL_TYPE, class TRUE_TYPE, bool IS_DYNAMIC_CAST = true>
    struct PolymorphicUniquePtrWrapper{

        std::unique_ptr<VIRTUAL_TYPE>& ptr;

        PolymorphicUniquePtrWrapper(std::unique_ptr<VIRTUAL_TYPE>& ptr): ptr(ptr){};

    };

    template <class T, class RegisteredClasses>
    struct AutoPolymorphicUniquePtrWrapper{

        std::unique_ptr<T>& ptr;

        AutoPolymorphicUniquePtrWrapper(std::unique_ptr<T>& ptr): ptr(ptr){};

    };

    template <class T, class SZ_TYPE>
    struct ParasiteLinearDataWrapper{

        T *& ptr;
        SZ_TYPE& n;

        ParasiteLinearDataWrapper(T *& ptr, SZ_TYPE& n): ptr(ptr), n(n){};

    };

    template <class T>
    struct ParasitePtrWrapper{

        T *& ptr

        ParasitePtrWrapper(T *& ptr): ptr(ptr){};

    };

    template <class T, unsigned LENGTH>
    class TupleSerializable{

        public:

            template <unsigned IDX>
            auto get(){

                return static_cast<T *>(this)->template get<IDX>(); 

            }

            TupleSerializable<T, LENGTH> * to_tuple_serializable(){

                return this;

            } 

    };

    template <class T>
    class ContainerIterable{

        public:

            auto next(){

                return static_cast<T *>(this)->next();    

            }

            bool is_eor(){

                return static_cast<T *>(this)->is_eor();

            }

    };

    template <class T>
    class ContainerSerializable{

        public:

            void reserve(size_t sz){

                static_cast<T *>(this)->reserve(sz);

            }

            template <class T1>
            void insert(T1& val){

                static_cast<T *>(this)->insert(val);

            }

            auto size(){

                return static_cast<T *>(this)->size();

            }

            auto get_iter(){

                return static_cast<T *>(this)->get_iter();
            }

            auto get_containee_type(){

                return static_cast<T *>(this)->get_containee_type();

            }

            ContainerSerializable<T> * to_container_serializable(){

                return this; 

            }

    };

    class CAllocator{

        public:

            template <class T>
            static inline T * malloc(size_t length){

                return reinterpret_cast<T *>(std::malloc(sizeof(T) * length));
            } 

            template <class T>
            static inline T * malloc(){

                return reinterpret_cast<T *>(std::malloc(sizeof(T)));

            }

            template <class T>
            static inline void free_linear(T * data){

                std::free(data);

            }
            
            template <class T>
            static inline void free(T * data){

                std::free(data);

            }

    };

    class CPPAllocator{

        public:

            template <class T>
            static inline T * malloc(size_t length){

                return new T[length];

            }

            template <class T>
            static inline T * malloc(){

                return new T;

            }

            template <class T>
            static inline void free_linear(T * data){

                delete[] data;

            }

            template <class T>
            static inline void free(T * data){

                delete data;

            }

    };

    using StdMallocPolicy = CAllocator;
    using NewDeletePolicy = CPPAllocator; 

};

namespace dg::serializer::pointer{

    template <class T>
    class SharedPtrGeneratorSingleton{

        private:

            static inline std::unordered_map<uintptr_t, std::shared_ptr<T>> data{};
        
        public:

            static inline std::shared_ptr<T>& get(T * ptr){

            }

            static inline void clear(){

            }

    };


};

namespace dg::serializer::pointer::static_pointer{

    template <class T>
    class PointerIDRegisterable{

        public:

            bool exists(void * ptr){

                return static_cast<T *>(this)->exists(ptr);

            }

            size_t get(void * ptr){

                return static_cast<T *>(this)->get(ptr);

            } 

            void insert(void * ptr, size_t id){

                return static_cast<T *>(this)->insert(ptr, id);

            }

    };

    template <class T>
    class PointerFreeVerifiable{

        public:

            bool is_freeable(void * ptr){

                return static_cast<T*>(this)->is_freeable(ptr);

            }

            void insert(void * ptr){

                static_cast<T*>(this)->insert(ptr);

            }

    };

    template <class T>
    class PointerAddrRetrievable{

        public:

            void * get(size_t id){

                return static_cast<T *>(this)->get(id);

            }

            void insert(size_t id, void * ptr){

                return static_cast<T *>(this)->insert(id, ptr);

            }

    };

    template <class T>
    class PointerRangeRegisterable{

        public:

            void insert(void * ptr, size_t range){

                static_cast<T*>(this)->insert(ptr, range);

            }

            void get_head(void * ptr){

                static_cast<T*>(this)->get_head(ptr);

            }

    };

    template <class T>
    class ParasitePointerVector{

        public:

            void insert(void * ptr){

                static_cast<T *>(this)->insert(ptr);

            }

            size_t size(){
                
                return static_cast<T *>(this)->size();

            }

            void * get(size_t idx){

                return static_cast<T *>(this)->get(idx);

            }

    };

    template <class T>
    class ParasitePointerFinalizable{

        public:

            void finalize(){

                static_cast<T *>(this)->finalize();

            }

    };

};

namespace dg::serializer::pointer::poly_pointer{

    using PolyClassIDType = uint32_t;
    static inline PolyClassIDType NOT_FOUND_ID = std::numeric_limits<PolyClassIDType>::max();

    template <class T>
    class DynamicTypeCallbackable{

        public:

            template <class T1, class ...Args>
            void callback(T1 * ptr, PolyClassIDType id, Args&& ...args){

                static_cast<T*>(this)->callback(ptr, id, std::forward<Args>(args)...);

            }

    };

    template <class T>
    class ForwardResolverControllable{

        public:

            template <class T1, class T2, class ...Args, class ...CB_Args>
            void resolve(T1 * ptr, DynamicTypeCallbackable<T2>& cb_obj, const PolyRegisteredClasses<Args...>& registered, CB_Args&& ...cb_args){

                static_cast<T *>(this)->resolve(ptr, cb_obj, registered, std::forward<CB_Args>(cb_args)...);

            }

    };

    template <class T>
    class BackwardResolverControllable{

        public:

            template <class T1, class T2, class ...Args, class ...CB_Args>
            void id_resolve(T1 * ptr, PolyClassIDType id, DynamicTypeCallbackable<T2>& cb_obj, const PolyRegisteredClasses<Args...>& registered, CB_Args&& ...cb_args){

                static_cast<T *>(this)->id_resolve(ptr, id, cb_obj, registered, std::forward<CB_Args>(cb_args)...);

            }

    };

};

namespace dg::serializer::std_wrapper{

    static inline const uint8_t VECTOR_ID = 0;
    static inline const uint8_t DEQUE_ID = 1;
    static inline const uint8_t UNORDERED_SET_ID = 2;
    static inline const uint8_t UNORDERED_MULTISET_ID = 3;
    static inline const uint8_t SET_ID = 4;
    static inline const uint8_t MULTISET_ID = 5;
    static inline const uint8_t UNORDERED_MAP_ID = 6;
    static inline const uint8_t UNORDERED_MULTIMAP_ID = 7;
    static inline const uint8_t MAP_ID = 8;
    static inline const uint8_t MULTIMAP_ID = 9;
    static inline const uint8_t STACK_ID = 10;
    static inline const uint8_t QUEUE_ID = 11;
    static inline const uint8_t PRIORITY_QUEUE = 12;

    template <class T>
    class IterWrapper: public ContainerIterable<IterWrapper<T>>{

        private:

            T cur_iter;
            T end_iter;

        public:

            KeyOnlyIterWrapper(T begin_iter, T end_iter): cur_iter(begin_iter), end_iter(end_iter){}

            auto next(){

                return std::ref(*(this->cur_iter++));

            }

            bool is_eor(){

                return this->cur_iter == this->end_iter;

            }

    };

    class IterWrapperGenerator{

        public:

            template <class T>
            static inline auto get(T iter_begin, T iter_end){
                
                return IterWrapper<T>(iter_begin, iter_end);

            }
    };

    template <class Container_Type, class Containee_Type>
    class StdKeyOnlyLikedInterfaceBase{

        private:

            Container_Type& data;
    
        public:

            StdKeyOnlyLikedInterfaceBase(Container_Type& data): data(data){}

            void reserve(size_t sz){

                this->data.reserve(sz);

            }

            size_t size(){

                return this->data->size();

            }

            auto get_iter(){

                return IterWrapperGenerator::get(this->data.begin(), this->data.end());

            }

            auto get_containee_type(){

                return Containee_Type();

            }

    };

    template <class Container_Type, class Containee_Type>
    class VectorLikedIntefaceWrapper: public ContainerSerializable<VectorLikedIntefaceWrapper<Container_Type, Containee_Type>>,
                                      private StdKeyOnlyLikedInterfaceBase<Container_Type, Containee_Type>{

        private:

            Container_Type& data;
        
        public:

            using Base = StdKeyOnlyLikedInterfaceBase<Container_Type, Containee_Type>;
            using Base::reserve;
            using Base::size;
            using Base::get_iter;
            using Base::get_containee_type; 

            VectorLikedIntefaceWrapper(Container_Type& data): data(data), Base(data){};

            template <class T1>
            void insert(T1&& val){

                this->data.push_back(std::forward<T1>(val));

            }


    };

    template <class Container_Type, class Containee_Type>
    class UnorderedSetLikedInterfaceWrapper: public ContainerSerializable<UnorderedSetLikedInterfaceWrapper<Container_Type, Containee_Type>>,
                                             private StdKeyOnlyLikedInterfaceBase<Container_Type, Containee_Type>{
        
        private:

            Containee_Type& data;
        
            public:

            using Base = StdKeyOnlyLikedInterfaceBase<Container_Type, Containee_Type>;
            using Base::reserve;
            using Base::size;
            using Base::get_iter;
            using Base::get_containee_type; 

            VectorLikedIntefaceWrapper(Container_Type& data): data(data), Base(data){};

            template <class T1>
            void insert(T1&& val){

                this->data.insert(std::forward<T1>(val));

            }

    };

    template <class Container_Type, class Key_Type, class Value_Type>
    class UnorderedMapLikedInterfaceWrapper: public ContainerSerializable<UnorderedMapWrapper<Container_Type, Key_Type, Value_Type>>{

        private:

            Container_Type& data;

        public:

            UnorderedMapLikedInterfaceWrapper(Container_Type& data): data(data){};

            void insert(std::pair<Key_Type, Value_Type>& kv_pair){

                this->data.insert(kv_pair);

            }

            void insert(std::pair<Key_Type, Value_Type>&& kv_pair){

                this->data.insert(std::move(kv_pair));

            }

            void reserve(size_t sz){

                this->data.reserve(sz);

            }

            size_t size(){

                return this->data.size();

            }
            
            auto get_iter(){

                return IterWrapperGenerator::get(this->data.begin(), this->data.end());

            }
            

            auto get_containee_type(){

                 return std::pair<Key_Type, Value_Type>();

            }

    };
    
    template <class _Deque_Container>
    struct ContainerAdopterDequeCast{

        template <class T>
        static inline _Deque_Container& get(T& data){

            static_assert(std::is_standard_layout<_Deque_Container>::value);
            static_assert(std::is_standard_layout<T>::value);

            return reinterpret_cast<_Deque_Container&>(*reinterpret_cast<_Deque_Container *>(&data)); 

        }

    };

    class StdContainerWrapperGenerator{

        public:

            using Self = StdContainerWrapperGenerator;

            template <class T, class T1>
            static auto get_vector(std::vector<T, T1>& data){

                return VectorLikedIntefaceWrapper<std::vector<T, T1>, T>(data);

            }

            template <class T, class T1>
            static auto get_deque(std::deque<T, T1>& data){

                return VectorLikedIntefaceWrapper<std::deque<T, T1>, T>(data);

            }

            template <class T, class T1, class T2, class T3>
            static auto get_unordered_set(std::unordered_set<T, T1, T2, T3>& data){

                return UnorderedSetLikedInterfaceWrapper<std::unordered_set<T, T1, T2, T3>, T>(data);
                
            }

            template <class T, class T1, class T2, class T3>
            static auto get_unordered_multiset(std::unordered_multiset<T, T1, T2, T3>& data){

                return UnorderedSetLikedInterfaceWrapper<std::unordered_multiset<T, T1, T2, T3>, T>(data);

            }

            template <class T, class T1, class T2>
            static auto get_set(std::set<T, T1, T2>& data){

                return UnorderedSetLikedInterfaceWrapper<std::set<T, T1, T2>, T>(data);

            }

            template <class T, class T1, class T2>
            static auto get_multiset(std::multiset<T, T1, T2>& data){

                return UnorderedSetLikedInterfaceWrapper<std::multiset<T, T1, T2>, T>(data);

            }

            template <class T, class T1, class T2, class T3, class T4>
            static auto get_unordered_map(std::unordered_map<T, T1, T2, T3, T4>& data){

                return UnorderedMapLikedInterfaceWrapper<std::unordered_map<T, T1, T2, T3, T4>, T, T1>(data);

            }

            template <class T, class T1, class T2, class T3, class T4>
            static auto get_unordered_multimap(std::unordered_multimap<T, T1, T2, T3, T4>& data){
                
                return UnorderedMapLikedInterfaceWrapper<std::unordered_multimap<T, T1, T2, T3, T4>, T, T1>(data);

            }

            template <class T, class T1, class T2, class T3>
            static auto get_map(std::map<T, T1, T2, T3>& data){

                return UnorderedMapLikedInterfaceWrapper<std::map<T, T1, T2, T3>, T, T1>(data);

            }

            template <class T, class T1, class T2, class T3>
            static auto get_multimap(std::map<T, T1, T2, T3>& data){

                return UnorderedMapLikedInterfaceWrapper<std::multimap<T, T1, T2, T3>, T, T1>(data);

            }

            template <class T, class _Container>
            static auto get_stack(std::stack<T, _Container>& data){

                _Container& dq = ContainerAdopterDequeCast<_Container>::get(data); 

                return Self::get_deque(dq); 

            }

            template <class T, class _Container>
            static auto get_queue(std::queue<T, _Container>& data){

                _Container& dq = ContainerAdopterDequeCast<_Container>::get(data);

                return Self::get_deque(dq);

            }

            template <class T, class _Container>
            static auto get_priority_queue(std::priority_queue<T, _Container>& data){

                _Container& dq = ContainerAdopterDequeCast<_Container>::get(data);

                return Self::get_deque(dq); 

            }

    };
    
    template <class T>
    struct StdContainerCheck{

        static inline const bool is_std_container = false;

    };

    template <class T, class T1>
    struct StdContainerCheck<std::vector<T, T1>>{

        static inline const bool is_std_container = true;
        static inline const uint8_t id = VECTOR_ID;

    };

    template <class T, class T1>
    struct StdContainerCheck<std::deque<T, T1>>{

        static inline const bool is_std_container = true;
        static inline const uint8_t id = DEQUE_ID;

    };

    template <class T, class T1, class T2, class T3>
    struct StdContainerCheck<std::unordered_set<T, T1, T2, T3>>{

        static inline const bool is_std_container = true;
        static inline const uint8_t id = UNORDERED_SET_ID;

    };

    template <class T, class T1, class T2, class T3>
    struct StdContainerCheck<std::unordered_multiset<T, T1, T2, T3>>{

        static inline const bool is_std_container = true;
        static inline const uint8_t id = UNORDERED_MULTISET_ID; 

    };

    template <class T, class T1, class T2>
    struct StdContainerCheck<std::set<T, T1, T2>>{
        
        static inline const bool is_std_container = true;
        static inline const uint8_t id = SET_ID;

    };

    template <class T, class T1, class T2>
    struct StdContainerCheck<std::multiset<T, T1, T2>>{

        static inline const bool is_std_container = true;
        static inline const uint8_t id = MULTISET_ID;

    };

    template <class T, class T1, class T2, class T3, class T4>
    struct StdContainerCheck<std::unordered_map<T, T1, T2, T3, T4>>{

        static inline bool is_std_container = true;
        static inline const uint8_t id = UNORDERED_MAP_ID;
    };

    template <class T, class T1, class T2, class T3, class T4>
    struct StdContainerCheck<std::unordered_multimap<T, T1, T2, T3, T4>>{

        static inline const bool is_std_container = true;
        static inline const uint8_t id = UNORDERED_MULTIMAP_ID;

    };

    template <class T, class T1, class T2, class T3>
    struct StdContainerCheck<std::map<T, T1, T2, T3>>{

        static inline const bool is_std_container = true;
        static inline const uint8_t id = MAP_ID;

    };

    template <class T, class T1, class T2, class T3>
    struct StdContainerCheck<std::multimap<T, T1, T2, T3>>{

        static inline const bool is_std_container = true;
        static inline const uint8_t id = MULTIMAP_ID;

    };

    template <class T, class T1>
    struct StdContainerCheck<std::stack<T, T1>>{

        static inline const bool is_std_container = true;
        static inline const uint8_t id = STACK_ID;

    };

    template <class T, class T1>
    struct StdContainerCheck<std::queue<T, T1>>{

        static inline const bool is_std_container = true;
        static inline const uint8_t id = QUEUE_ID;

    };

    template <class T, class T1, class T2>
    struct StdContainerCheck<std::priority_queue<T, T1, T2>>{

        static inline const bool is_std_container = true;
        static inline const uint8_t id = PRIORITY_QUEUE;

    };

    class StdContainerUtility{

        public:

            template <class T>
            static constexpr bool is_std_container(){

                return StdContainerCheck<T>::is_std_container;

            };

            template <class T>
            static auto wrap(T& std_container){

                if constexpr(StdContainerCheck<T>::id == VECTOR_ID){

                    return StdContainerWrapperGenerator::get_vector(std_container);

                } else if constexpr(StdContainerCheck<T>::id == DEQUE_ID){

                    return StdContainerWrapperGenerator::get_deque(std_container);

                } else if constexpr(StdContainerCheck<T>::id == UNORDERED_SET_ID){

                    return StdContainerWrapperGenerator::get_unordered_set(std_container);

                } else if constexpr(StdContainerCheck<T>::id == UNORDERED_MULTISET_ID){

                    return StdContainerWrapperGenerator::get_unordered_multiset(std_container);

                } else if constexpr(StdContainerCheck<T>::id == SET_ID){

                    return StdContainerWrapperGenerator::get_set(std_container);

                } else if constexpr(StdContainerCheck<T>::id == MULTISET_ID){

                    return StdContainerWrapperGenerator::get_multiset(std_container);

                } else if constexpr(StdContainerCheck<T>::id == UNORDERED_MAP_ID){

                    return StdContainerWrapperGenerator::get_unordered_map(std_container);

                } else if constexpr(StdContainerCheck<T>::id == UNORDERED_MULTIMAP_ID){

                    return StdContainerWrapperGenerator::get_unordered_multimap(std_container);

                } else if constexpr(StdContainerCheck<T>::id == MAP_ID){

                    return StdContainerWrapperGenerator::get_map(std_container);

                } else if constexpr(StdContainerCheck<T>::id == MULTIMAP_ID){

                    return StdContainerWrapperGenerator::get_multimap(std_container);

                } else if constexpr(StdContainerCheck<T>::id == STACK_ID){

                    return StdContainerWrapperGenerator::get_stack(std_container);

                } else if constexpr(StdContainerCheck<T>::id == QUEUE_ID){

                    return StdContainerWrapperGenerator::get_queue(std_container);

                } else if constexpr(StdContainerCheck<T>::id == PRIORITY_QUEUE){

                    return StdContainerWrapperGenerator::get_priority_queue(std_container);

                } else{

                    assert(false);

                }

            }

    };

    class StdContainerTwoWayArchiver: public Archivable<StdContainerTwoWayArchiver>{

        public:

            template <class T, class T1>
            void put(Archivable<T>& obj_archive, T1&& val){
                
                using stripped_type_ref = typename dgstd::remove_reference<T1>::type&;

                auto wrapped = StdContainerUtility::wrap((stripped_type_ref) val);
                obj_archive.put(obj_archive, wrapped);

            }

            template <class T>
            constexpr bool is_archivable() const{

                return StdContainerUtility::is_std_container<T>(); 

            }

    };

    class MultiDataTypeUtility{

        public:

            template <class ...Args>
            static inline constexpr unsigned count(){

            } 

            template <class ...Args>
            static inline constexpr unsigned inc_count(){

                return count<Args...>() + 1;

            }

    };

    class EOR{};

    template <class T, class ...Args>
    class MultiDataTypeWrapper: public TupleSerializable<MultiDataTypeWrapper<T, Args...>, MultiDataTypeUtility::inc_count<Args...>()>,
                                public MultiDataTypeWrapper<Args...>{

        private:

            T * cur;

        public:

            using Interface = TupleSerializable<MultiDataTypeWrapper<T, Args...>, MultiDataTypeUtility::inc_count<Args...>()>;
            using Base = MultiDataTypeWrapper<Args...>;
            using Interface::to_tuple_serializable;

            MultiDataTypeWrapper(): cur(nullptr), Base(){};

            MultiDataTypeWrapper(T& cur, Args&& ...args): cur(&cur), Base(std::forward<Args>(args)...){};

            template <unsigned IDX>
            auto get(){
                
                constexpr unsigned cur_idx = MultiDataTypeUtility::count<Args...>();

                if (constexpr(IDX == cur_idx)){

                    return std::ref(*this->cur);

                } else{

                    return Base::get<IDX>();

                }

            }

            auto& init(T& cur){

                this->cur = &cur;

                return static_cast<Base&>(*this);

            }

    };

    template <class T>
    class MultiDataTypeWrapper: public TupleSerializable<MultiDataTypeWrapper<T>, 1>,
                                public EOR{

        private:

            T * cur;
        
        public:

            MultiDataTypeWrapper(): cur(nullptr){};

            MultiDataTypeWrapper(T& cur): cur(&cur){};

            template <unsigned IDX>
            auto get(){

                static_assert(IDX == 0);

                return std::ref(*this->cur); 

            }

            auto& init(T& cur){

                this->cur = &cur;

                return static_cast<EOR&>(*this);

            }

    };

    template <unsigned LENGTH>
    struct init_tup{

        template <class ...Args, class ...Args1>
        void init(std::tuple<Args...>& data, MultiDataTypeWrapper<Args1...>& rs){

            init_tup<LENGTH - 1>(data, rs.init(std::get<LENGTH - 1>(data)));

        }

    };

    template <>
    struct init_tup<0>{

        template <class T, class T1>
        void init(T&&, T1&&){}

    };

    class MultiDataTypeWrapperGenerator{

        public:

            template <class T, class T1>
            static inline MultiDataTypeWrapper<T, T1> get_pair(std::pair<T, T1>& data){

                return MultiDataTypeWrapper<T, T1>(data.first, data.second);

            }

            template <class ...Args>
            static inline MultiDataTypeWrapper<Args...> get_tup(std::tuple<Args...>& data){
                
                MultiDataTypeWrapper<Args...> rs;
                MultiDataTypeWrapperGenerator::init_tup(data, rs);

                return rs;

            }

    };
    
    template <class T>
    struct StdPairChecker{

        static inline const bool is_std_pair = false;

    };

    template <class T, class T1>
    struct StdPairChecker<std::pair<T, T1>>{

        static inline const bool is_std_pair = true;

    };

    class StdPairTwoWayArchiver: public Archivable<StdPairTwoWayArchiver>{

        public:

            template <class T, class T1>
            void put(Archivable<T>& obj_archive, T1&& data){

                using stripped_type_ref = typename dgstd::remove_reference<T>::type&;

                this->put_helper(obj_archive, reinterpret_cast<stripped_type_ref>(data)); 

            }

            template <class T>
            constexpr bool is_archivable() const{

                return StdPairChecker<T>::is_std_pair;  

            }

        private:

            template <class T, class T1, class T2>
            void put_helper(Archivable<T>& obj_archive, std::pair<T1, T2>& data){

                auto wrapped_pair = MultiDataTypeWrapperGenerator::get_pair(data);
                obj_archive.put(obj_archive, *wrapped_pair.to_tuple_serializable());

            }

    };

    template <class T>
    struct StdTupleChecker{

        static inline bool is_std_tuple = false;

    };

    template <class ...Args>
    struct StdTupleChecker<std::tuple<Args...>>{

        static inline bool is_std_tuple = true;

    };

    class StdTupleTwoWayArchiver: public Archivable<StdTupleTwoWayArchiver>{

        public:

            template <class T, class T1>
            void put(Archivable<T>& obj_archive, T1&& data){

                using stripped_type_ref = dgstd::remove_reference<T1>::type&;

                this->put_helper(obj_archive, reinterpret_cast<stripped_type_ref>(data));

            }

            template <class T>
            constexpr bool is_archivable() const{
                
                return StdTupleChecker<T>::is_std_tuple;

            }

        private:

            template <class T, class ...Args>
            void put_helper(Archivable<T>& obj_archive, std::tuple<Args...>& tup){

                auto tup_wrapper = MultiDataTypeWrapperGenerator::get_tup(tup);
                obj_archive.put(obj_archive, *tup_wrapper.to_tuple_serializable());

            }

    };

};

namespace dg::serializer{

    class PODUtility{

        public:

            template <unsigned WIDTH>
            auto get_type(){

                if constexpr(WIDTH == 1){

                    return uint8_t{};

                } else if constexpr(WIDTH == 2){

                    return uint16_t{};

                } else if constexpr(WIDTH == 4){

                    return uint32_t{};

                } else if constexpr(WIDTH == 8){

                    return uint64_t{};

                }

            }

    };

    template <class T, class T1>
    class PODForwardArchiver: public Archivable<PODForwardArchiver<T, T1>>{

        private:

            std::shared_ptr<ByteStreamable<T1>> streamer;
        
        public:

            PODForwardArchiver(std::shared_ptr<ByteStreamable<T1>> streamer){

                this->streamer = streamer;

            }

            template <class T2, class T3>
            void put(Archivable<T2>&, T3&& val){
                
                using stripped_type = typename dgstd::remove_reference<T3>::type;
                
                this->streamer->add(reinterpret_cast<stripped_type>(val));

            }

            template <class T2>
            constexpr bool is_archivable() const{

                return std::is_pod<T2>::value;

            }

    };

    template <class T, class T1>
    class PODBackwardArchiver: public Archivable<PODBackwardArchiver<T, T1>>{

        private:

            std::shared_ptr<ByteSequenceIterable<T>> iter;
            std::shared_ptr<DataInitializable<T1>> initializer;
        
        public:

            PODBackwardArchiver(std::shared_ptr<ByteSequenceIterable<T>> iter, std::shared_ptr<DataInitializable<T1>> initialier){

                this->iter = iter;
                this->initializer = initializer;

            }

            template <class T2, class T3>
            void put(Archivable<T2>&, T3&& val){
                
                using stripped_type = typename dgstd::remove_reference<T3>::type;
                using stripped_type_ref = stripped_type&;

                auto bseq = this->iter->template next_<sizeof(stripped_type)>();
                this->initializer->init(reinterpret_cast<stripped_type_ref>(val), bseq);

            }

            template <class T2>
            constexpr bool is_archivable() const{

                return std::is_pod<T2>::value;

            }

    };

    template <class T>
    struct TupleCheck{

        static inline const bool is_tuple_serializable = false;

    };

    template <class T, unsigned LENGTH>
    struct TupleCheck<TupleSerializable<T, LENGTH>>{

        static inline const bool is_tuple_serializable = true;

    };

    class TupleUtility{

        public:

            template <class T>
            static constexpr bool is_tuple_serializable(){

                return TupleCheck<T>::is_tuple_serializable;

            };

    };

    class TupleTwoWayArchiver: public Archivable<TupleTwoWayArchiver>{

        public:

            template <class T, class T1>
            void put(Archivable<T>& obj_archive, T1&& val){
                
                using stripped_type_ref = typename dgstd::remove_reference<T1>::type&;

                this->put_init(obj_archive, reinterpret_cast<stripped_type_ref>(val));

            }

            template <class T>
            constexpr bool is_archivable() const{

                return TupleUtility::is_tuple_serializable<T>();

            }

        private:

            template <class T, class T1, unsigned LENGTH, unsigned CUR_LENGTH>
            void rec_put(Archivable<T>& obj_archive, TupleSerializable<T1, LENGTH>& data){

                if constexpr(CUR_LENGTH != 0){

                    obj_archive.put(obj_archive, data.template get<CUR_LENGTH - 1>());
                    this->rec_put<T, T1, LENGTH, CUR_LENGTH - 1>(obj_archive, data);

                }

            }

            template <class T, class T1, unsigned LENGTH>
            void put_init(Archivable<T>& obj_archive, TupleSerializable<T1, LENGTH>& data){

                this->rec_put<T, T1, LENGTH, LENGTH>(obj_archive, data);

            }

    };

    template <class T>
    struct ContainerCheck{

        static inline const bool is_container_serializable = false;

    };

    template <class T>
    struct ContainerCheck<ContainerSerializable<T>>{
        
        static inline const bool is_container_serializable = true;

    };

    class ContainerUtility{

        public:

            template <class T>
            static constexpr bool is_container_serializable(){

                return ContainerCheck<T>::is_container_serializable;

            }  

            using SZ_TYPE = size_t; 

    };

    class ContainerForwardArchiver: public Archivable<ContainerForwardArchiver>{

        public:

            template <class T, class T1>
            void put(Archivable<T>& obj_archive, T1&& data){

                using stripped_data_ref = typename dgstd::remove_reference<T1>::type&;

                this->put_helper(obj_archive, reinterpret_cast<stripped_data_ref>(data));

            }

            template <class T>
            constexpr bool is_archivable() const{
                
                return ContainerUtility::is_container_serializable<T>();

            }

        private:

            template <class T, class T1>
            void put_helper(Archivable<T>& obj_archive, ContainerSerializable<T1>& container){

                constexpr auto transformable_obj = Transformable<T2>();
                typename ContainerUtility::SZ_TYPE sz = container.size();

                obj_archive.put(obj_archive, sz);
                auto iter = container.get_iter();
                this->put_iter(obj_archive, iter, transformable_obj);

            }

            template <class T, class T1>
            void put_iter(Archivable<T>& obj_archive, ContainerIterable<T1>& iter){

                while (!iter.is_eor()){

                    obj_archive.put(obj_archive, iter.next());
                
                }

            }

    };

    class ContainerBackwardArchiver: public Archivable<ContainerBackwardArchiver>{

        public:

            template <class T, class T1>
            void put(Archivable<T>& obj_archive, T1&& data){

                using stripped_data_ref = typename dgstd::remove_reference<T1>::type&;

                this->put_helper(obj_archive, reinterpret_cast<stripped_data_ref>(data));

            }

            template <class T>
            constexpr bool is_archivable() const{

                return ContainerUtility::is_container_serializable<T>();

            }

        private:

            template <class T, class T1>
            void put_helper(Archivable<T>& obj_archive, ContainerSerializable<T1>& container){

                typename ContainerUtility::SZ_TYPE sz{};
                obj_archive.put(obj_archive, sz);

                for (size_t i = 0; i < sz; ++i){

                    decltype(container.get_containee_type()) containee; 
                    obj_archive.put(obj_archive, containee);                        
                    container.insert(std::move(containee));

                }

            }

    };

    template <class T>
    struct SerializableChecker{
        
        static inline const bool is_serializable = false;

    };
    
    template <class T>
    struct SerializableChecker<Serializable<T>>{

        static inline const bool is_serializable = true;

    };

    class SerializableObjectTwoWayArchiver: public Archivable<SerializableObjectTwoWayArchiver>{

        public:

            template <class T, class T1>
            void put(Archivable<T>& obj_archive, T1&& data){

                using stripped_type_ref = typename dgstd::remove_reference<T1>::type&;

                this->put_helper(obj_archive, reinterpret_cast<stripped_type_ref>(data));

            }

            template <class T>
            constexpr bool is_archivable() const{

                return SerializableChecker<T>::is_serializable;

            }
        
        private:

            template <class T, class T1>
            void put_helper(Archivable<T>& obj_archive, Serializable<T1>& obj){

                obj.serialize(obj_archive);

            }

    };

    class EOL{}; 

    template <class T, class ...Args>
    class TwoWayLinkedArchive: public TwoWayLinkedArchive<Args...>,
                               private Archivable<TwoWayLinkedArchive<T, Args...>>{

        private:

            Archivable<T>& cur;

        public:

            using Base = TwoWayLinkedArchive<Args...>;
            using Interface = Archivable<TwoWayLinkedArchive<T, Args...>>;
            using Interface::to_archivable;

            TwoWayLinkedArchive(Archivable<T>& cur, Args&& ...args): cur(cur), Base(std::forward<Args>(args)...){}

            template <class T1, class T2, class ...Args>
            void put(Archivable<T1>& obj_archive, T2&& data, Args&& ...args){

                if constexpr(Archivable<T>(). template is_archivable<dgstd::remove_reference<T2>::type>()){

                    this->cur.put(obj_archive, std::forward<T2>(data));

                } else{

                    Base::put(obj_archive, std::forward<T2>(data));

                }

                this->put(obj_archive, std::forward<Args>(args)...);

            } 

            template <class T1>
            void put(Archivable<T1>&){}

            template <class T1>
            constexpr bool is_archivable() const{

                return is_archivable_static<T1>();

            }

            template <class T1>
            static constexpr bool is_archivable_static() const{

                return Archivable<T>().template is_archivable<T1>() || Base::is_archivable_static<T1>(); 

            }


    };

    template <>
    class TwoWayLinkedArchive<EOL>{

        public:

            template <class T, class T1>
            void put(Archivable<T>&, T1&&){

                assert(false);

            }

            template <class T>
            void put(Archivable<T>&){}

            template <class T1>
            constexpr bool is_archivable() const{
                
                return false;

            }

    };

};

namespace dg::serializer::pointer::static_pointer{

    template <class T>
    class StdPointerIDRegister: public PointerIDRegisterable<StdPointerIDRegister<T>>{

        private:

            std::unordered_map<uintptr_t, size_t> ptr_id_map;

        public:

            StdPointerIDRegister(): id_ptr_map(){}

            bool exists(void * ptr){

                auto iter = this->ptr_id_map.find(reinterpret_cast<uintptr_t>(ptr));

                return iter != this->ptr_id_map.end(); 

            } 

            size_t get(void * ptr){
                
                auto iter = this->ptr_id_map.find(reinterpret_cast<uintptr_t>(ptr));
                assert(iter != this->ptr_id_map.end());

                return (*iter).second; 

            }

            void insert(void * ptr, size_t id){
                
                assert(!this->exists(ptr));

                this->ptr_id_map.insert(std::make_pair(reinterpret_cast<uintptr_t>(ptr), id));

            }

    };

    template <class T>
    class StdPointerAddrRetriever: public PointerAddrRetrievable<StdPointerAddrRetriever<T>>{

        private:

            std::unordered_map<size_t, uintptr_t> id_ptr_map;
        
        public:

            StdPointerAddrRetriever(): id_ptr_map(){}

            void * get(size_t id){

                auto iter = this->id_ptr_map.find(id);
                assert(iter != this->id_ptr_map.end());
                
                return reinterpret_cast<void *>((*iter).second);

            }

            void insert(size_t id, void * addr){

                assert((this->id_ptr_map.find(id) == this->id_ptr_map.end()));

                this->id_ptr_map.insert(std::make_pair(id, reinterpret_cast<uintptr_t>(addr)));

            }   

    };

    template <class T>
    class StdPointerFreeVerifier: public PointerFreeVerifiable<StdPointerFreeVerifier<T>>{

        private:

            std::unordered_set<uintptr_t> freed_addrs;
        
        public:

            StdPointerFreeVerifier(): freed_addrs(){};

            bool is_freeable(void * ptr){

                return this->freed_addrs.find(reinterpret_cast<uintptr_t>(ptr)) == this->freed_addrs.end(); 

            }

            void insert(void * ptr){

                this->freed_addrs.insert(reinterpret_cast<uintptr_t>(ptr));
                
            }

    };

    template <class T>
    struct StablePointerCheck{

        static inline bool is_stable_ptr = false;

    };

    template <class T, class T1>
    struct StablePointerCheck<StablePtrWrapper<T, T1>>{

        static inline bool is_stable_ptr = true;

    };

    class StablePointerUtility{

        public:

            typedef uint8_t HEADER_TYPE;
            typedef size_t PTR_ID_TYPE; 

            static inline const HEADER_TYPE NULLPTR_HEADER = 0;
            static inline const HEADER_TYPE INITIAL_PTR_HEADER = 1;
            static inline const HEADER_TYPE REUSE_PTR_HEADER = 2; 

            template <class T>
            static constexpr bool is_stable_ptr() {

                return StablePointerCheck<T>::is_stable_ptr;

            }

            template <class T, class _Allocator_Policy>
            static void * to_void_ptr(StablePtrWrapper<T, _Allocator_Policy>& data){

                return reinterpret_cast<void *>(data.ptr); 

            }

            template <class T, class _Allocator_Policy>
            static size_t get_pointee_size(StablePtrWrapper<T, _Allocator_Policy>&){

                return sizeof(T);
                
            }

            template <class T, class _Allocator_Policy, class T1>
            static void pointer_assign(StablePtrWrapper<T, _Allocator_Policy>& data, T1 * ptr_data){

                data.ptr = reinterpret_cast<T *>(ptr_data);

            }

            template <class T, class _Allocator_Policy>
            static T * get_pointer_value(StablePtrWrapper<T, _Allocator_Policy>& data){

                return data.ptr;

            }
            
            template <class T, class _Allocator_Policy>
            static T& deref(StablePtrWrapper<T, _Allocator_Policy>& cur){

                return *cur.ptr;

            }

            template <class T, std::enable_if_t<std::is_integral<dgstd::remove_reference<T>>::value, bool> = true>
            static PTR_ID_TYPE cast_id(T&& val){

                return val;

            }

    };

    template <class T, class T1>
    class StablePointerForwardArchiver: public Archivable<StablePointerForwardArchiver<T, T1>>{

        private:

            std::shared_ptr<PointerIDRegisterable<T>> pointer_id_register;
            std::shared_ptr<ByteSequenceCountable<T1>> counter;

        public:

            StablePointerForwardArchiver(std::shared_ptr<PointerIDRegisterable<T>> pointer_id_register,
                                         std::shared_ptr<ByteSequenceCountable<T1>> counter){
                
                this->pointer_id_register = pointer_id_register;
                this->counter = counter;

            }

            template <class T2, class T3>
            void put(Archivable<T2>& obj_archive, T3&& data){

                using stripped_type_ref = typename dgstd::remove_reference<T3>&;

                this->put_helper(obj_archive, (stripped_type_ref) data);

            }

            template <class T2>
            constexpr bool is_archivable() const{
                
                return StablePointerUtility::is_stable_ptr<T2>(); 

            }

        private:

            template <class T2, class T3, class _Allocator_Policy>
            void put_helper(Archivable<T2>& obj_archive, StablePtrWrapper<T3, _Allocator_Policy>& data){

                void * void_casted_ptr = StablePointerUtility::to_void_ptr(data); 

                if (void_casted_ptr == nullptr){

                    obj_archive.put(obj_archive, StablePointerUtility::NULLPTR_HEADER);

                } else{

                    if (this->pointer_id_register->exists(void_casted_ptr)){

                        obj_archive.put(obj_archive, StablePointerUtility::REUSE_PTR_HEADER);
                        obj_archive.put(obj_archive, StablePointerUtility::cast_id(this->pointer_id_register->get(void_casted_ptr)));

                    } else{

                        obj_archive.put(obj_archive, StablePointerUtility::INITIAL_PTR_HEADER);
                        this->pointer_id_register->insert(void_casted_ptr, this->counter->size());
                        obj_archive.put(obj_archive, StablePointerUtility::deref(data));

                    }

                }

            }

    };

    template <class T, class T1, class T2>
    class StablePointerBackwardArchiver: public Archivable<StablePointerBackwardArchiver<T, T1, T2>>{

        private:

            std::shared_ptr<PointerAddrRetrievable<T>> ptr_retriever;
            std::shared_ptr<ByteSequenceCountable<T1>> counter;
            std::shared_ptr<PointerFreeVerifiable<T2>> free_verifier;

        public:

            StablePointerBackwardArchiver(std::shared_ptr<PointerAddrRetrievable<T>> ptr_retriever, 
                                          std::shared_ptr<ByteSequenceCountable<T1>> counter,
                                          std::shared_ptr<PointerFreeVerifiable<T2>> free_verifier){

                this->ptr_retriever = ptr_retriever;
                this->counter = counter;
                this->free_verifier = free_verifier;

            }

            template <class T3, class T4>
            void put(Archivable<T3>& obj_archive, T4&& data){

                using stripped_type_ref = typename dgstd::remove_reference<T4>::type&;

                this->put_helper(obj_archive, (stripped_type_ref) data);

            }

            template <class T3>
            constexpr bool is_archivable() const{
                
                return StablePointerUtility::is_stable_ptr<T3>(); 

            }

        private:

            template <class T3, class T4, class _Allocator_Policy>
            void put_helper(Archivable<T3>& obj_archive, StablePtrWrapper<T4, _Allocator_Policy>& data){

                StablePointerUtility::HEADER_TYPE header{};
                StablePointerUtility::PTR_ID_TYPE id{};

                void * void_casted_ptr = StablePointerUtility::to_void_ptr(data);

                if (this->free_verifier->is_freeable(void_casted_ptr)){
                    
                    _Allocator_Policy::free(StablePointerUtility::get_pointer_value(data));
                    this->free_verifier->insert(void_casted_ptr);

                }

                obj_archive.put(obj_archive, header);

                if (header == StablePointerUtility::NULLPTR_HEADER){

                    StablePointerUtility::pointer_assign(data, nullptr);

                } else if (header == StablePointerUtility::REUSE_PTR_HEADER){
                    
                    obj_archive.put(obj_archive, id);
                    StablePointerUtility::pointer_assign(data, this->ptr_retriever->get(id));

                } else if (header == StablePointerUtility::INITIAL_PTR_HEADER){
                        
                    id = this->counter->count();
                    StablePointerUtility::pointer_assign(data, _Allocator_Policy::malloc<T4>());
                    this->ptr_retriever->insert(id, StablePointerUtility::to_void_ptr(data));

                    obj_archive.put(obj_archive, StablePointerUtility::deref(data));

                } else{

                    assert(false);

                }
                
            }

    };

    template <class T>
    struct StableLinearChecker{

        static inline bool is_stable_linear_serializable = false;

    };
    
    template <class T, class SZ_TYPE, class _Allocator_Policy>
    struct StableLinearChecker<StableLinearDataWrapper<T, SZ_TYPE, _Allocator_Policy>>{

        static inline bool is_stable_linear_serializable = true;
        
        using Containee_Type = T; 

    };

    class StableLinearDataUtility{

        public:

            typedef uint8_t HEADER_TYPE;
            typedef size_t PTR_ID_TYPE; 

            static inline const HEADER_TYPE NULLPTR_HEADER = 0;
            static inline const HEADER_TYPE INITIAL_PTR_HEADER = 1;
            static inline const HEADER_TYPE REUSE_PTR_HEADER = 2; 
            
            template <class T>
            static constexpr bool is_stable_linear_serializable(){

                return StableLinearChecker<T>::is_stable_linear_serializable;

            }

            template <class T>
            static constexpr bool is_pod_stable_linear_serializable(){

                if constexpr(StableLinearChecker<T>::is_stable_linear_serializable){

                    return std::id_pod<typename StableLinearChecker<T>::Containee_Type>::value;

                } else{

                    return false;

                }

            }

            template <class T, class SZ_TYPE, class _Allocator_Policy>
            static void * to_void_ptr(StableLinearDataWrapper<T, SZ_TYPE, _Allocator_Policy>& data){

                return reinterpret_cast<void *>(data.ptr);

            }
            
            template <class T, class SZ_TYPE, class _Allocator_Policy>
            static T * get_ptr_value(StableLinearDataWrapper<T, SZ_TYPE, _Allocator_Policy>& data){

                return reinterpret_cast<T *>(data.ptr);

            }

            template <class T, class SZ_TYPE, class _Allocator_Policy>
            static SZ_TYPE get_size_value(StableLinearDataWrapper<T, SZ_TYPE, _Allocator_Policy>& data){

                return reinterpret_cast<SZ_TYPE>(data.n); 

            }

            template <class T, class SZ_TYPE, class _Allocator_Policy>
            static size_t get_pointee_size(StableLinearDataWrapper<T, SZ_TYPE, _Allocator_Policy>&){

                return sizeof(T);
                
            }

            template <class T, class SZ_TYPE, class _Allocator_Policy>
            static SZ_TYPE& get_size_ref(StableLinearDataWrapper<T, SZ_TYPE, _Allocator_Policy>& data){

                return data.n; 

            }

            template <class T, class SZ_TYPE, class _Allocator_Policy>
            static T& deref(StableLinearDataWrapper<T, SZ_TYPE, _Allocator_Policy>& data, size_t idx){

                return reinterpret_cast<T&>(data.ptr[idx]);

            }

            template <class T>
            static PTR_ID_TYPE cast_id(T&& val){
                
                return reinterpret_cast<PTR_ID_TYPE>(val);

            }

    };

    template <class T>
    class LinearPODContentForwardArchiver: public Archivable<LinearPODContentForwardArchiver<T>>{

        private:

            std::shared_ptr<LinearStreamable<T>> linear_streamer;
        
        public:

            LinearPODContentForwardArchiver() = default;

            LinearPODContentForwardArchiver(std::shared_ptr<LinearStreamable<T>> linear_streamer){

                this->linear_streamer = linear_streamer;

            }

            template <class T2, class T3>
            void put(Archivable<T2>& obj_archive, T3&& data){

                using stripped_type_ref = typename dgstd::remove_reference<T3>::type&;

                this->put_helper(obj_archive, reinterpret_cast<stripped_type_ref>(data));

            }

            template <class T2>
            constexpr bool is_archivable() const{

                return StableLinearDataUtility::is_pod_stable_linear_serializable<T2>(); 

            }

        private:

            template <class T2, class T3, class SZ_TYPE, class _Allocator_Policy, std::enable_if_t<std::is_pod<T3>::value, bool> = true>
            void put_helper(Archivable<T2>& obj_archive, StableLinearDataWrapper<T3, SZ_TYPE, _Allocator_Policy>& data){
                
                this->linear_streamer->add_linear(StableLinearDataUtility::get_ptr_value(data), StableLinearDataUtility::get_size_value(data));

            }

    };

    class StdLinearContentForwardArchiver: public Archivable<StdLinearContentForwardArchiver>{

        public:

            template <class T, class T1>
            void put(Archivable<T>& obj_archive, T1&& data){

                using stripped_type_ref = typename dgstd::remove_reference<T1>::type&;

                this->put_helper(obj_archive, reinterpret_cast<stripped_type_ref>(data));

            }

            template <class T>
            constexpr bool is_archivable() const{

                return StableLinearDataUtility::is_stable_linear_serializable<T>();

            }
        
        private:

            template <class T, class T1, class SZ_TYPE, class _Allocator_Policy>
            void put_helper(Archivable<T>& obj_archive, StableLinearDataWrapper<T1, SZ_TYPE, _Allocator_Policy>& data){

                for (size_t i = 0; i < StableLinearDataUtility::get_size_value(data); ++i){

                    obj_archive.put(obj_archive, StableLinearDataUtility::deref(data, i));

                }

            }

    };

    template <class T, class T1, class T2>
    class StableLinearDataForwardArchiver: public Archivable<StableLinearDataForwardArchiver<T, T1, T2>>,
                                           private LinearPODContentForwardArchiver<T2>,
                                           private StdLinearContentForwardArchiver{

        private:

            std::shared_ptr<PointerIDRegisterable<T>> ptr_id_register;
            std::shared_ptr<ByteSequenceCountable<T1>> counter;
        
        public:

            using LinearPOD = LinearPODContentForwardArchiver<T2>;
            using StdContent = StdLinearContentForwardArchiver;
            using Interface = Archivable<StableLinearDataForwardArchiver<T, T1, T2>>;
            using Interface::to_archivable;

            StableLinearDataForwardArchiver(std::shared_ptr<PointerIDRegisterable<T>> ptr_id_register,
                                            std::shared_ptr<ByteSequenceCountable<T1>> counter,
                                            std::shared_ptr<LinearStreamable<T2>> linear_streamer): LinearPOD(linear_streamer), 
                                                                                                    StdContent(){
                
                this->ptr_id_register = ptr_id_register;
                this->counter = counter;

            }

            template <class T3, class T4>
            void put(Archivable<T3>& obj_archive, T4&& data){

                using stripped_type_ref = typename dgstd::remove_reference<T4>::type&;

                this->put_helper(obj_archive, reinterpret_cast<stripped_type_ref>(data));

            }

            template <class T3>
            constexpr bool is_archivable() const{

                return StableLinearDataUtility::is_stable_linear_serializable<T3>();  

            }

        private:

            template <class T3, class T4, class SZ_TYPE, class _Allocator_Policy>
            void put_helper(Archivable<T3>& obj_archive, StableLinearDataWrapper<T4, SZ_TYPE, _Allocator_Policy>& wrapper){
                
                using wrapper_type = StableLinearDataWrapper<T4, SZ_TYPE, _Allocator_Policy>;

                obj_archive.put(obj_archive, StableLinearDataUtility::get_size_value(wrapper));
                void * void_casted_ptr = StableLinearDataUtility::to_void_ptr(wrapper);

                if (void_casted_ptr == nullptr){

                    obj_archive.put(obj_archive, StableLinearDataUtility::NULLPTR_HEADER);

                } else{

                    if (this->ptr_id_register->exists(void_casted_ptr)){

                        obj_archive.put(obj_archive, StableLinearDataUtility::REUSE_PTR_HEADER);
                        obj_archive.put(obj_archive, StableLinearDataUtility::cast_id(this->ptr_id_register.get(void_casted_ptr)));

                    } else{

                        obj_archive.put(obj_archive, StableLinearDataUtility::INITIAL_PTR_HEADER);
                        this->ptr_id_register->insert(void_casted_ptr, this->counter->size());

                        if constexpr(LinearPOD().template is_archivable<wrapper_type>()){

                            LinearPOD::put(obj_archive, wrapper);

                        } else{

                            StdContent::put(obj_archive, wrapper);

                        }   

                    }

                }

            }

    };

    template <class T, class T1, class T2>
    class StableLinearDataBackwardArchiver: public Archivable<StableLinearDataBackwardArchiver<T, T1, T2>>{

        private:

            std::shared_ptr<PointerAddrRetrievable<T>> ptr_retriever;
            std::shared_ptr<ByteSequenceCountable<T1>> counter;
            std::shared_ptr<PointerFreeVerifiable<T2>> free_verifier;

        public:

            StableLinearDataBackwardArchiver(std::shared_ptr<PointerAddrRetrievable<T>> ptr_retriever,
                                             std::shared_ptr<ByteSequenceCountable<T1>> counter,
                                             std::shared_ptr<PointerFreeVerifiable<T2>> free_verifier){
                
                this->ptr_retriever = ptr_retriever;
                this->counter = counter;
                this->free_verifier = free_verifier;

            }

            template <class T3, class T4>
            void put(Archivable<T3>& obj_archive, T4&& data){

                using stripped_type_ref = typename dgstd::remove_reference<T4>::type&;

                this->put_helper(obj_archive, reinterpret_cast<stripped_type_ref>(data)); 

            }

            template <class T3>
            constexpr bool is_archivable() const{
                
                return StableLinearDataUtility::is_stable_linear_serializable<T3>();

            }

        private:

            template <class T3, class T4, class SZ_TYPE, class _Allocator_Policy>
            void put_helper(Archivable<T3>& obj_archive, StableLinearDataWrapper<T4, SZ_TYPE, _Allocator_Policy>& data){
                
                StableLinearDataUtility::HEADER_TYPE header{};
                StableLinearDataUtility::PTR_ID_TYPE id{};
                void * void_casted_ptr = StableLinearDataUtility::to_void_ptr(data); 

                if (this->free_verifier->is_freeable(void_casted_ptr)){

                    _Allocator_Policy::free_linear(StableLinearDataUtility::get_ptr_value(data));
                    this->free_verifier->insert(void_casted_ptr);

                }

                obj_archive.put(obj_archive, StableLinearDataUtility::get_size_ref(data));
                obj_archive.put(obj_archive, header);

                if (header == StableLinearDataUtility::NULLPTR_HEADER){

                    StableLinearDataUtility::assign_pointer(data, nullptr);

                } else if (header == StableLinearDataUtility::REUSE_PTR_HEADER){

                    obj_archive.put(obj_archive, id);
                    StableLinearDataUtility::assign_ptr(data, this->ptr_retriever->get(id));

                } else if (header == StableLinearDataUtility::INITIAL_PTR_HEADER){
                    
                    size_t sz_val = StableLinearDataUtility::get_size_value(data);
                    StableLinearDataUtility::assign_ptr(data, _Allocator_Policy::malloc<T4>(sz_val));
                    this->ptr_retriever->insert(StableLinearDataUtility::to_void_ptr(data), this->counter->size());

                    for (size_t i = 0; i < sz_val; ++i){

                        obj_archive.put(obj_archive, StableLinearDataUtility::deref(data, i));

                    }

                } else{

                    assert(false);

                }

            }

    };

    template <class T>
    class SmartPointerCheck{

        static inline const bool is_smart_ptr = false;

    };

    template <class T>
    class SmartPointerCheck<SharedPtrWrapper<T>>{

        static inline const bool is_smart_ptr = true;

    };

    template <class T>
    class SmartPointerCheck<UniquePtrWrapper<T>>{

        static inline const bool is_smart_ptr = true;

    };

    class SmartPointerUtility{
        
        public:

            template <class T>
            static constexpr bool is_smart_ptr(){

                return SmartPointerCheck<T>::is_smart_ptr;

            }
            
    };

    class StdSmartPointerForwardArchiver: public Archivable<StdSmartPointerForwardArchiver>{
        
        public:

            using Self = StdSmartPointerForwardArchiver;

            template <class T, class T1>
            void put(Archivable<T>& obj_archive, T1&& data){

                using stripped_data_ref = typename dgstd::remove_reference<T1>::type&;

                this->put_helper(obj_archive, reinterpret_cast<stripped_data_ref>(data)); 

            }

            template <class T>
            constexpr bool is_archivable() const{
            
                return SmartPointerUtility::is_smart_ptr<T>(); 

            }
            

        private:

            template <class T, class T1>
            void put_helper(Archivable<T>& obj_archive, SharedPtrWrapper<T1>& data){
                
                T1 * ptr_value = reinterpret_cast<T1 *>(&*data.ptr);

                StablePtrWrapper<T1, NewDeletePolicy> wrapper(ptr_value);
                obj_archive.put(obj_archive, wrapper);
                
            };

            template <class T, class T1>
            void put_helper(Archivable<T>& obj_archive, UniquePtrWrapper<T1>& data){
                
                T1 * ptr_value = reinterpret_cast<T1 *>(&*data.ptr);

                StablePtrWrapper<T1, NewDeletePolicy> wrapper(ptr_value);
                obj_archive.put(obj_archive, wrapper);

            }


    };

    class StdSmartPointerBackwardArchiver: public Archivable<StdSmartPointerBackwardArchiver>{
        
        public:

            template <class T, class T1>
            void put(Archivable<T>& obj_archive, T1&& data){

                using stripped_type_ref = typename dgstd::remove_reference<T1>::type&;

                this->put_helper(obj_archive, reinterpret_cast<stripped_type_ref>(data));

            }

            template <class T>
            constexpr bool is_archivable() const{
                
                return SmartPointerUtility().is_smart_ptr<T>(); 

            }

        private:

            template <class T, class T1>
            void put_helper(Archivable<T>& obj_archive, SharedPtrWrapper<T1>& data){
                
                T1 * raw_ptr = nullptr;
                StablePtrWrapper<T1, NewDeletePolicy> wrapper(raw_ptr);

                obj_archive.put(obj_archive, wrapper);
                data.ptr = SharedPtrGeneratorSingleton<T1>::get(raw_ptr);

            }

            template <class T, class T1>
            void put_helper(Archivable<T>& obj_archive, UniquePtrWrapper<T1>& data){

                T1 * raw_ptr = nullptr;
                StablePtrWrapper<T1, NewDeletePolicy> wrapper(raw_ptr);

                obj_archive.put(obj_archive, wrapper);
                data.ptr = std::unique_ptr<T1>(raw_ptr); 

            }

    };

    template <class T>
    class StdPointerRangeRegister: public PointerRangeRegisterable<StdPointerRangeRegister<T>>{

        private:

            std::set<std::pair<uintptr_t, size_t>, std::greater<std::pair<uintptr_t, size_t>>> data;
        
        public:

            StdPointerRangeRegister(): data(){}

            void insert(void * ptr, size_t range){
                
                data.insert(std::make_pair(reinterpret_cast<uintptr_t>(ptr), range));

            }

            void * get_head(void * ptr){

                std::pair<uintptr_t, size_t> key = std::make_pair(reinterpret_cast<uintptr_t>(ptr), std::numeric_limits<size_t>::max());
                auto iter = data.lower_bound(key);

                assert(iter != data.end());

                std::pair<uintptr_t, size_t>& true_key = *iter;

                assert(true_key.first + true_key.second >= reinterpret_cast<uintptr_t>(ptr));

                return reinterpret_cast<void *>(true_key.first);

            }

    };

    template <class T, class T1, class T2, class T3>
    class RangeRegisteredStableLinearDataForwardArchiver: public Archivable<RangeRegisteredStableLinearDataForwardArchiver<T, T1, T2, T3>>,
                                                          private StableLinearDataForwardArchiver<T, T1, T2>{

        private:

            std::shared_ptr<PointerRangeRegisterable<T3>> range_register;

        public:

            using Base = StableLinearDataForwardArchiver<T, T1, T2>;
            using Base::is_archivable;
            using Interface = Archivable<RangeRegisteredStableLinearDataForwardArchiver<T, T1, T2, T3>>;
            using Interface::to_archivable;

            template <class T4, class T5>
            void put(Archivable<T4>& obj_archive, T5&& data){

                using stripped_type_ref = typename dgstd::remove_reference<T1>::type&;

                void * casted_void_ptr = StableLinearDataUtility::to_void_ptr(reinterpret_cast<stripped_type_ref>(data));
                size_t range = StableLinearDataUtility::get_pointee_size(reinterpret_cast<stripped_type_ref>(data)) * StableLinearDataUtility::get_size_value(reinterpret_cast<stripped_type_ref>(data));

                this->range_register->insert(casted_void_ptr, range);

                Base::put(obj_archive, std::forward<T5>(data));

            }

    };

    template <class T, class T1, class T2>
    class RangeRegisteredStablePointerForwardArchiver: public Archivable<RangeRegisteredStableLinearDataForwardArchiver<T, T1, T2>>,
                                                       private StablePointerForwardArchiver<T, T1>{
        
        private:

            std::shared_ptr<PointerRangeRegisterable<T2>> range_register;
        
        public:

            using Base = StablePointerForwardArchiver<T, T1>;
            using Base::is_archivable;
            using Interface = Archivable<RangeRegisteredStableLinearDataForwardArchiver<T, T1, T2>>;
            using Interface::to_archivable;

            template <class T3, class T4>
            void put(Archivable<T3>& obj_archive, T4&& data){

                using stripped_type_ref = typename dgstd::remove_reference<T1>::type&;

                void * casted_void_ptr = StablePointerUtility::to_void_ptr(reinterpret_cast<stripped_type_ref>(data));
                size_t range = StablePointerUtility::get_pointee_size(reinterpret_cast<stripped_type_ref>(data));

                this->range_register->insert(casted_void_ptr, range);

                Base::put(obj_archive, std::forward<T4>(data));

            }

    };

    template <class T>
    struct ParasitePointerChecker{

        static inline bool is_parasite_pointer = false;

    };
    
    template <class T>
    struct ParasitePointerChecker<ParasitePtrWrapper<T>>{

        static inline bool is_parasite_pointer = true;

    };

    class ParasitePointerUtility{

        public:

            typedef size_t OFFSET_TYPE; 
            typedef size_t ID_TYPE;

            template <class T>
            static void * to_void_ptr(ParasitePtrWrapper<T>& data){

                return reinterpret_cast<void *>(data.ptr);

            }

            template <class T>
            static void * to_ptr_void_ptr(ParasitePtrWrapper<T>& data){

                return reinterpret_cast<void *>(&data.ptr);

            } 

            template <class T>
            static bool is_parasite_pointer(){

                return ParasitePointerChecker<T>::is_parasite_pointer; 

            } 

            static void assign_pointer(void ** ptr_ptr, void * ptr){

                (*ptr_ptr) = ptr;

            }

    };

    template <class T>
    class ParasitePointerForwardArchiver: public Archivable<ParasitePointerForwardArchiver<T>>{

        private:

            std::shared_ptr<ParasitePointerVector<T>> register_vector;
        
        public:

            ParasitePointerForwardArchiver(std::shared_ptr<ParasitePointerVector<T>> register_vector): register_vector(register_vector){}

            template <class T1, class T2>
            void put(Archivable<T1>&, T2&& data){

                using stripped_type_ref = typename dgstd::remove_reference<T2>::type&;

                this->register_vector->insert(ParasitePointerUtility::to_void_ptr(reinterpret_cast<stripped_type_ref>(data)));

            }

            template <class T1>
            constexpr bool is_archivable() const{

                return ParasitePointerUtility::is_parasite_pointer<T1>(); 

            }

    };

    template <class T, class T1, class T2, class T3>
    class ParasitePointerForwardFinalizer: public ParasitePointerFinalizable<ParasitePointerForwardFinalizer<T, T1, T2, T3>>{

        private:

            std::shared_ptr<ParasitePointerVector<T>> register_vector;
            std::shared_ptr<PointerRangeRegisterable<T1>> stable_addr_register;
            std::shared_ptr<ByteStreamable<T2>> op_stream;
            std::shared_ptr<PointerIDRegisterable<T3>> ptr_id_register;
        
        public:

            ParasitePointerForwardFinalizer(std::shared_ptr<ParasitePointerVector<T>> register_vector,
                                            std::shared_ptr<PointerRangeRegisterable<T1>> stable_addr_register,
                                            std::shared_ptr<ByteStreamable<T2>> op_stream,
                                            std::shared_ptr<PointerIDRegisterable<T3>> ptr_id_register){
                
                this->register_vector = register_vector;
                this->stable_addr_register = stable_addr_register;
                this->op_stream = op_stream;
                this->ptr_id_register = ptr_id_register;

            }

            void finalize(){

                void * parasite_addr = nullptr;
                void * head_addr = nullptr;
                typename ParasitePointerUtility::OFFSET_TYPE offset{}; 
                typename ParasitePointerUtility::ID_TYPE id{};

                for (size_t i = 0; i < register_vector->size(); ++i){

                    parasite_addr = register_vector->get(i);
                    head_addr = this->stable_addr_register->get_head(parasite_addr);
                    offset = (uintptr_t) parasite_addr - (uintptr_t) head_addr;
                    id = this->ptr_id_register->get(head_addr);

                    this->op_stream.add(id);
                    this->op_stream.add(offset);

                }
            }

    };

    template <class T>
    class ParasitePointerBackwardArchiver: public Archivable<ParasitePointerBackwardArchiver<T>>{

        private:

            std::shared_ptr<ParasitePointerVector<T>> register_vector;
        
        public:

            ParasitePointerBackwardArchiver(std::shared_ptr<ParasitePointerVector<T>> register_vector): register_vector(register_vector){}

            template <class T1, class T2>
            void put(Archivable<T1>&, T2&& data){

                using stripped_type_ref = typename dgstd::remove_reference<T2>::type&;

                this->register_vector->insert(ParasitePointerUtility::to_ptr_void_ptr(reinterpret_cast<stripped_type_ref>(data)));

            }

            template <class T1>
            constexpr bool is_archivable() const{
                
                return ParasitePointerUtility::is_parasite_pointer<T1>();

            }

    };

    template <class T, class T1, class T2, class T3>
    class ParasitePointerBackwardFinalizer: public ParasitePointerFinalizable<ParasitePointerBackwardArchiver<T, T1, T2, T3>>{

        private:

            std::shared_ptr<ParasitePointerVector<T>> register_vector;
            std::shared_ptr<PointerAddrRetrievable<T1>> ptr_addr_retriever;
            std::shared_ptr<ByteSequenceIterable<T2>> bseq_iter;
            std::shared_ptr<DataInitializable<T3>> data_initilizer;

        public:

            ParasitePointerBackwardFinalizer(std::shared_ptr<ParasitePointerVector<T>> register_vector,
                                             std::shared_ptr<PointerAddrRetrievable<T1>> ptr_addr_retriever,
                                             std::shared_ptr<ByteSequenceIterable<T2>> bseq_iter,
                                             std::shared_ptr<DataInitializable<T3>> data_initilizer){
                
                this->register_vector = register_vector;
                this->ptr_addr_retriever = ptr_addr_retriever;
                this->bseq_iter = bseq_iter;
                this->data_initilizer = data_initilizer;

            }

            void finalize(){

                typename ParasitePointerUtility::ID_TYPE cur_id{};
                typename ParasitePointerUtility::OFFSET_TYPE cur_offset{};
                void * head_ptr = nullptr;
                void * cur_ptr = nullptr; 

                for (size_t i = 0; i < this->register_vector->size(); ++i){

                    auto id_bseq = this->bseq_iter->template next_<sizeof(typename ParasitePointerUtility::ID_TYPE)>();
                    auto offset_bseq = this->bseq_iter->template next_<sizeof(typename ParasitePointerUtility::OFFSET_TYPE)>();

                    this->data_initilizer->init(cur_id, id_bseq);
                    this->data_initilizer->init(cur_offset, offset_bseq);

                    head_ptr = this->ptr_addr_retriever->get(cur_id);
                    cur_ptr = reinterpret_cast<void *>((char *) head_ptr + cur_offset);

                    ParasitePointerUtility::assign_pointer(reinterpret_cast<void**>(this->register_vector->get(i)), cur_ptr);

                }

            }

            
    };

};

namespace dg::serializer::pointer::poly_pointer{

    class EOR{};

    template <class Base, class Derived, class NEXT_>
    struct RecursiveInheritanceInfo{

        static constexpr PolyClassIDType id = NEXT_::id + 1;

    };

    template <class Base, class Derived>
    struct RecursiveInheritanceInfo<Base, Derived, EOR>{

        static constexpr PolyClassIDType id = 1;

    };

    template <class NEXT_BASE, class NEXT_DERIVED>
    struct Joiner{

        template <class Base, class Derived, class NEXT_>
        constexpr auto join(RecursiveInheritanceInfo<Base, Derived, NEXT_>){

            return RecursiveInheritanceInfo<NEXT_BASE, NEXT_DERIVED, RecursiveInheritanceInfo<Base, Derived, NEXT_>>();

        }

        constexpr auto join(EOR){

            return RecursiveInheritanceInfo<NEXT_BASE, NEXT_DERIVED, EOR>();

        }

    };

    template <class CUR, class ...Args>
    struct Aggregator{

        template <class Base, class Derived, class NEXT_, class Cur_Inheritance>
        constexpr auto traverse_n_join(RecursiveInheritanceInfo<Base, Derived, NEXT_>, Cur_Inheritance){
            
            auto joined = Joiner<Base, Derived>().join(Cur_Inheritance());

            return traverse_n_join(NEXT_(), joined);

        }
        
        template <class Cur_Inheritance>
        constexpr auto traverse_n_join(EOR, Cur_Inheritance){

            return Cur_Inheritance();

        }

        constexpr auto aggregate(){

            auto base_aggregated = Aggregator<Args...>().aggregate();

            return traverse_n_join(CUR(), base_aggregated);

        }

    };

    template <class CUR>
    struct Aggregator<CUR>{
        
        constexpr auto aggregate(){

            return CUR();

        }

    };

    template <class CUR, class NEXT, class ...Args>
    struct TraverseParser{

        constexpr auto parse(){

            using next_parser = TraverseParser<CUR, Args...>;
            auto tmp = next_parser().parse();

            if constexpr(std::is_base_of<CUR, NEXT>::value){

                return Joiner<CUR, NEXT>().join(tmp);

            } else{

                return tmp;

            }

        }

        constexpr auto backward_parse(){

            using next_parser = TraverseParser<CUR, Args...>;
            auto tmp = next_parser().backward_parse();

            if constexpr(std::is_base_of<NEXT, CUR>::value){

                return Joiner<NEXT, CUR>().join(tmp);

            } else{

                return tmp;

            }

        }

    };

    template <class CUR, class NEXT>
    struct TraverseParser<CUR, NEXT>{

        constexpr auto parse(){

            if constexpr(std::is_base_of<CUR, NEXT>::value){

                return RecursiveInheritanceInfo<CUR, NEXT, EOR>();

            } else{

                return EOR();

            }

        }

        constexpr auto backward_parse(){

            if constexpr(std::is_base_of<NEXT, CUR>::value){

                return RecursiveInheritanceInfo<NEXT, CUR, EOR>();

            } else{

                return EOR();

            }

        }
    };

    template <class CUR, class ...Args>
    struct InheritanceParser{

        constexpr auto parse(){

            using next_parser = InheritanceParser<Args...>;
            using traverse_parser = TraverseParser<CUR, Args...>; 

            auto tmp = next_parser().parse(); 
            auto fwd_lst = traverse_parser().parse();
            auto bwd_lst = traverse_parser().backward_parse();

            return Aggregator<decltype(tmp), decltype(fwd_lst), decltype(bwd_lst)>().aggregate(); 


        };

    };

    template <class CUR>
    struct InheritanceParser<CUR>{

        constexpr auto parse(){

            return EOR();

        }

    };

    template <class Key_Base>
    struct BaseSpecificInheritanceGetter{

        public:

            template <class Base, class Derived, class NEXT_>
            constexpr auto get(RecursiveInheritanceInfo<Base, Derived, NEXT_>){

                return BaseSpecificInheritanceGetter<Key_Base>().get(NEXT_());

            }

            template <class Derived, class NEXT_>
            constexpr auto get(RecursiveInheritanceInfo<Key_Base, Derived, NEXT_>){

                auto rs = BaseSpecificInheritanceGetter<Key_Base>().get(NEXT_());
                
                return Joiner<Key_Base, Derived>().join(rs); 

            }

            constexpr auto get(EOR){

                return EOR();

            }

    };

    template <class Base>
    struct TransitiveGenerator{

        public:

            template <class Intermediate, class Derived, class NEXT_>
            constexpr auto get(RecursiveInheritanceInfo<Intermediate, Derived, NEXT_>){

                auto rs = TransitiveGenerator<Base>().get(NEXT_());

                return Joiner<Base, Derived>().join(rs);

            }

            constexpr auto get(EOR){

                return EOR();

            }

    };

    template <class InheritanceInfo>
    struct TransitiveInheritanceGetter{

        public:

            template <class Base, class Derived, class NEXT_>
            constexpr auto get(RecursiveInheritanceInfo<Base, Derived, NEXT_>){

                auto inter_inheritance = BaseSpecificInheritanceGetter<Derived>().get(InheritanceInfo());
                auto trans_inheritance = TransitiveGenerator<Base>().get(inter_inheritance);
                auto rs = TransitiveInheritanceGetter<InheritanceInfo>().get(NEXT_());

                return Aggregator<decltype(rs), decltype(trans_inheritance)>().aggregate();

            }

            constexpr auto get(EOR){

                return EOR();

            }

    };

    template <class Base, class Derived>
    struct InheritanceFinder{
        
        template <class _Base, class _Derived, class NEXT_>
        constexpr bool is_in(RecursiveInheritanceInfo<_Base, _Derived, NEXT_>){

            return InheritanceFinder<Base, Derived>().is_in(NEXT_()); 

        }

        template <class NEXT_>
        constexpr bool is_in(RecursiveInheritanceInfo<Base, Derived, NEXT_>){

            return true;

        }

        constexpr bool is_in(EOR){

            return false;

        }

    };

    struct InheritancePruner{

        public:

            template <class Base, class Derived, class NEXT_, class T1>
            constexpr auto difference(RecursiveInheritanceInfo<Base, Derived, NEXT_>, T1){

                auto rs = InheritancePruner().difference(NEXT_(), T1());

                if constexpr(InheritanceFinder<Base, Derived>().is_in(T1())){

                    return rs;

                } else{

                    return Joiner<Base, Derived>().join(rs); 

                }
    
            }

            template <class T1>
            constexpr auto difference(EOR, T1){

                return EOR();

            }

    };

    struct StdInheritanceGenerator{

        template <class ...Args>
        constexpr auto get(PolyRegisteredClasses<Args...>){

            auto pre_pruned = InheritanceParser<Args...>().parse();
            auto transitive_list = TransitiveInheritanceGetter<decltype(pre_pruned)>().get(pre_pruned);
            auto pruned = InheritancePruner().difference(pre_pruned, transitive_list);

            return pruned;

        }

    };

    template <class T>
    struct IS_EOR{

        static inline const bool value = false;

    };

    template <>
    struct IS_EOR<EOR>{

        static inline const bool value = true;

    };

    template <class T, template <class> class ResolverInitializer, class InheritanceInfo, class BaseType, class DerivedType, class NEXT_>    
    struct DynamicTypeResolver{

        using Self = DynamicTypeResolver<T, ResolverInitializer, InheritanceInfo, BaseType, DerivedType, NEXT_>;

        template <class NEXT_BASE, class NEXT_DERIVED, class NEXT_NEXT>
        constexpr auto get_next_resolver(RecursiveInheritanceInfo<NEXT_BASE, NEXT_DERIVED, NEXT_NEXT>){

            return DynamicTypeResolver<T, ResolverInitializer, InheritanceInfo, NEXT_BASE, NEXT_DERIVED, NEXT_NEXT>();

        }

        template <class T1, class ...CB_Args>
        bool type_resolve(T * ptr, DynamicTypeCallbackable<T1>& cb_obj, CB_Args&& ...cb_args) const{

            if constexpr(!IS_EOR<NEXT_>::value){

                constexpr auto next_resolver = Self().get_next_resolver(NEXT_());

                return next_resolver.type_resolve(ptr, cb_obj, std::forward<CB_Args>(cb_args)...);

            } else{

                return false;

            }

        }  

    };

    template <class T, template <class> class ResolverInitializer, class InheritanceInfo, class DerivedType, class NEXT_>
    struct DynamicTypeResolver<T, ResolverInitializer, InheritanceInfo, T, DerivedType, NEXT_>{

        using Self = DynamicTypeResolver<T, ResolverInitializer, InheritanceInfo, T, DerivedType, NEXT_>;

        template <class NEXT_BASE, class NEXT_DERIVED, class NEXT_NEXT>
        constexpr auto get_next_resolver(RecursiveInheritanceInfo<NEXT_BASE, NEXT_DERIVED, NEXT_NEXT>){

            return DynamicTypeResolver<T, ResolverInitializer, InheritanceInfo, NEXT_BASE, NEXT_DERIVED, NEXT_NEXT>(); 

        }

        template <class T1, class ...CB_Args>
        bool type_resolve(T * ptr, DynamicTypeCallbackable<T1>& cb_obj, CB_Args&& ...cb_args) const{

            DerivedType * derived_ptr = dynamic_cast<DerivedType *>(ptr);

            if (derived_ptr == nullptr){

                if constexpr(!IS_EOR<NEXT_>::value){
                    
                    constexpr auto next_resolver = Self().get_next_resolver(NEXT_());

                    return next_resolver.type_resolve(ptr, cb_obj, std::forward<CB_Args>(cb_args)...);

                } else{

                    return false;

                }

            } else{
                
                constexpr auto resolver = ResolverInitializer<DerivedType>().get(InheritanceInfo());

                if (!resolver.type_resolve(derived_ptr, cb_obj, std::forward<CB_Args>(cb_args)...)){

                    auto cur_id = RecursiveInheritanceInfo<T, DerivedType, NEXT_>::id; 
                    cb_obj.callback(derived_ptr, cur_id, std::forward<CB_Args>(cb_args)...);

                }

                return true;

            }

        }

    };

    struct IDResolver{

        template <class Base, class Derived, class NEXT_, class T1, class ...CB_Args>
        void resolve(RecursiveInheritanceInfo<Base, Derived, NEXT_>, DynamicTypeCallbackable<T1>& cb_obj, PolyClassIDType id, CB_Args&& ...cb_args) const{

            if (RecursiveInheritanceInfo<Base, Derived, NEXT_>::id == id){

                cb_obj.callback((Derived *){}, id, std::forward<CB_Args>(cb_args)...);

            } else{

                resolve(NEXT_(), cb_obj, id, std::forward<CB_Args>(cb_args)...);

            }

        }

        template <class ...Args>
        void resolve(EOR, Args&&...) const{

            assert(false);

        }

    };

    template <class T>
    struct ResolverInitializer{

        template <class Base, class Derived, class NEXT_>
        constexpr auto get(RecursiveInheritanceInfo<Base, Derived, NEXT_>){

            return DynamicTypeResolver<T, ResolverInitializer, RecursiveInheritanceInfo<Base, Derived, NEXT_>, Base, Derived, NEXT_>();

        }

    };

    class StdDynamicTypeResolverController: public ForwardResolverControllable<StdDynamicTypeResolverController>,
                                            public BackwardResolverControllable<StdDynamicTypeResolverController>{

        public:

            template <class T, class T1, class ...Args, class ...CB_Args>
            void resolve(T * ptr, DynamicTypeCallbackable<T1>& cb_obj, const PolyRegisteredClasses<Args...>&, CB_Args&& ...cb_args){
                
                constexpr auto inheritance_info = StdInheritanceGenerator().get(PolyRegisteredClasses<Args...>());
                constexpr auto rec_resolver = ResolverInitializer<T>().get(inheritance_info);

                if (!rec_resolver.type_resolve(ptr, cb_obj, std::forward<CB_Args>(cb_args)...)){

                    cb_obj.callback(ptr, NOT_FOUND_ID, std::forward<CB_Args>(cb_args)...);

                }

            } 

            template <class T, class T1, class ...Args, class ...CB_Args>
            void id_resolve(T * ptr, PolyClassIDType id, const DynamicTypeCallbackable<T1>& cb_obj, PolyRegisteredClasses<Args...>&, CB_Args&& ...cb_args){

            constexpr auto inheritance_info = StdInheritanceGenerator().get(PolyRegisteredClasses<Args...>());
            constexpr auto id_resolver = IDResolver();

            if (id == NOT_FOUND_ID){

                cb_obj.callback(ptr, NOT_FOUND_ID, std::forward<CB_Args>(cb_args)...);

            } else{

                id_resolver.resolve(inheritance_info, cb_obj, id, std::forward<CB_Args>(cb_args)...);

            }

        }

    };

    template <class T>
    struct StableAutoPolymorphicPtrWrapperCheck{

        static inline const bool value = false;

    };

    template <class T, class ...Args, class _Allocator_Policy>
    struct StableAutoPolymorphicPtrWrapperCheck<StableAutoPolymorphicPtrWrapper<T, PolyRegisteredClasses<Args...>, _Allocator_Policy>>{

        static inline const bool value = true;

    };
    
    struct StableAutoPolyStablePtrUtility{

        template <class T>
        static constexpr bool is_auto_poly_stable_ptr(){

            return StableAutoPolymorphicPtrWrapperCheck<T>::value;

        }

        template <class T, class ...Args, class _Allocator_Policy>
        static T * get_ptr_value(StableAutoPolymorphicPtrWrapper<T, PolyRegisteredClasses<Args...>, _Allocator_Policy>& data){

            return reinterpret_cast<T *>(data.ptr);

        }

        template <class T, class ...Args, class _Allocator_Policy>
        static T *& get_ptr_ref(StableAutoPolymorphicPtrWrapper<T, PolyRegisteredClasses<Args...>, _Allocator_Policy>& data){

            return data.ptr; 

        }

        template <class T, class ...Args, class _Allocator_Policy, class T1>
        static void assign_ptr(StableAutoPolymorphicPtrWrapper<T, PolyRegisteredClasses<Args...>, _Allocator_Policy>& data, T1 * ptr){

            data.ptr = dynamic_cast<T*>(ptr);

        }

    };

    template <class T>
    class StableAutoPolymorphicPtrForwardArchiver: public Archivable<StableAutoPolymorphicPtrForwardArchiver<T>>,
                                                   public DynamicTypeCallbackable<StableAutoPolymorphicPtrForwardArchiver<T>>{

        private:

            std::shared_ptr<ForwardResolverControllable<T>> forward_resolver;
        
        public:

            StableAutoPolymorphicPtrForwardArchiver(std::shared_ptr<ForwardResolverControllable<T>> forward_resolver){
                
                this->forward_resolver = forward_resolver;

            }

            template <class T1, class T2>
            void put(Archivable<T1>& obj_archive, T2&& data){

                using stripped_type_ref = typename dgstd::remove_reference<T2>::type&;

                this->put_helper(obj_archive, reinterpret_cast<stripped_type_ref>(data));

            }

            template <class T1, class T2, class _Allocator_Policy>
            void callback(T1 * ptr, PolyClassIDType id, Archivable<T2>& obj_archive, const _Allocator_Policy&){

                StablePtrWrapper<T1, _Allocator_Policy> wrapper(ptr);
                
                obj_archive.put(obj_archive, id);
                obj_archive.put(obj_archive, wrapper);

            }

            template <class T2>
            constexpr bool is_archivable() const{

                return StableAutoPolyStablePtrUtility::is_auto_poly_stable_ptr<T2>();

            }

        private:

            template <class T1, class T2, class ...Args, class _Allocator_Policy>
            void put_helper(Archivable<T1>& obj_archive, StableAutoPolymorphicPtrWrapper<T2, PolyRegisteredClasses<Args...>, _Allocator_Policy>& wrapper){
                
                constexpr auto classes = PolyRegisteredClasses<Args...>();
                constexpr auto allocator = _Allocator_Policy();

                T2 * ptr = StableAutoPolyStablePtrUtility::get_ptr_value(wrapper);
                
                this->forward_resolver->resolve(ptr, *this, classes, obj_archive, allocator);

            }
    };

    template <class T>
    class StableAutoPolymorphicPtrBackwardArchiver: public Archivable<StableAutoPolymorphicPtrBackwardArchiver<T>>,
                                                    public DynamicTypeCallbackable<StableAutoPolymorphicPtrBackwardArchiver<T>>{

        private:

            std::shared_ptr<BackwardResolverControllable<T>> backward_resolver;
        
        public:

            StableAutoPolymorphicPtrBackwardArchiver(std::shared_ptr<BackwardResolverControllable<T>> backward_resolver){

                this->backward_resolver = backward_resolver;

            }

            template <class T1, class T2>
            void put(Archivable<T1>& obj_archive, T2&& data){

                using stripped_type_ref = dgstd::remove_reference<T2>::type&;

                this->put_helper(obj_archive, reinterpret_cast<stripped_type_ref>(data));

            }

            template <class T1, class T2, class T3, class ...Args, class _Allocator_Policy>
            void callback(T1 *, 
                          PolyClassIDType, 
                          Archivable<T2>& obj_archive, 
                          StableAutoPolymorphicPtrWrapper<T3, PolyRegisteredClasses<Args...>, _Allocator_Policy>& wrapper){
                
                T1 * deserialized_ptr = nullptr; 

                StablePtrWrapper<T1, _Allocator_Policy> stable_wrapper(deserialized_ptr);  
                obj_archive.put(obj_archive, stable_wrapper);
                StableAutoPolyStablePtrUtility::assign_ptr(wrapper, deserialized_ptr);

            }

            template <class T1>
            constexpr bool is_archivable() const{

                return StableAutoPolyStablePtrUtility::is_auto_poly_stable_ptr<T1>(); 

            }

        private:
            
            template <class T1, class T2, class ...Args, class _Allocator_Policy>
            void put_helper(Archivable<T1>& obj_archive, StableAutoPolymorphicPtrWrapper<T2, PolyRegisteredClasses<Args...>, _Allocator_Policy>& wrapper){
                
                constexpr auto classes = PolyRegisteredClasses<Args...>();
                PolyClassIDType id{};
                T2 * ptr = nullptr; 

                obj_archive.put(obj_archive, id);

                this->backward_resolver->id_resolve(ptr, id, *this, classes, obj_archive, wrapper);

            }

    };

    template <class T>
    struct SmartPointerCheck{

        static inline const bool value = false;

    };

    template <class T, class T1>
    struct SmartPointerCheck<AutoPolymorphicSharedPtrWrapper<T, T1>>{

        static inline const bool value = true;

    };

    template <class T, class T1>
    struct SmartPointerCheck<AutoPolymorphicUniquePtrWrapper<T, T1>>{

        static inline const bool value = true;

    };

    struct StableAutoPolySmartPtrUtility{

        template <class T>
        static constexpr bool is_auto_poly_smart_ptr(){

            return SmartPointerCheck<T>::value;

        }

        template <class T, class T1>
        static T * get_ptr_value(AutoPolymorphicSharedPtrWrapper<T, T1>& wrapper){

            return reinterpret_cast<T *>(&*wrapper.ptr);

        }

        template <class T, class T1>
        static T * get_ptr_value(AutoPolymorphicUniquePtrWrapper<T, T1>& wrapper){
            
            return reinterpret_cast<T *>(&*wrapper.ptr);

        }

        template <class T, class T1, class T2>
        static void assign_ptr(AutoPolymorphicSharedPtrWrapper<T, T1>& wrapper, T2 * ptr){
            
            std::shared_ptr<T2>& ptr_ = SharedPtrGeneratorSingleton<T2>::get(ptr);
            wrapper.ptr = std::dynamic_pointer_cast<T>(ptr_);

        }

        template <class T, class T1, class T2>
        static void assign_ptr(AutoPolymorphicUniquePtrWrapper<T, T1>& wrapper, T2 * ptr){

            wrapper.ptr = std::unique_ptr<T>(dynamic_cast<T *>(ptr));

        }

    };

    class StableAutoPolymorphicSmartPtrForwardArchiver: public Archivable<StableAutoPolymorphicSmartPtrForwardArchiver>{

        public:

            template <class T, class T1>
            void put(Archivable<T>& obj_archive, T1&& val){

                using stripped_type_ref = dgstd::remove_reference<T1>::type&;

                this->put_helper(obj_archive, reinterpret_cast<stripped_type_ref>(val)); 

            }

            template <class T>
            constexpr bool is_archivable() const{
                
                return StableAutoPolySmartPtrUtility::is_auto_poly_smart_ptr<T>();

            }

        private:

            template <class T, class T1, class RegisteredClasses>
            void put_helper(Archivable<T>& obj_archive, AutoPolymorphicSharedPtrWrapper<T1, RegisteredClasses>& wrapper){

                T1 * ptr_value = StableAutoPolySmartPtrUtility::get_ptr_value(wrapper);
                StableAutoPolymorphicPtrWrapper<T1, RegisteredClasses, NewDeletePolicy> wrapper_wrapper(ptr_value);
                obj_archive.put(obj_archive, wrapper_wrapper);

            }
            
            template <class T, class T1, class RegisteredClasses>
            void put_helper(Archivable<T>& obj_archive, AutoPolymorphicUniquePtrWrapper<T1, RegisteredClasses>& wrapper){

                T1 * ptr_value = StableAutoPolySmartPtrUtility::get_ptr_value(wrapper);
                StableAutoPolymorphicPtrWrapper<T1, RegisteredClasses, NewDeletePolicy> wrapper_wrapper(ptr_value);
                obj_archive.put(obj_archive, wrapper_wrapper);

            }

    };
    
    template <class T>
    class StableAutoPolymorphicSmartPtrBackwardArchiver: public Archivable<StableAutoPolymorphicSmartPtrBackwardArchiver<T>>,
                                                         public DynamicTypeCallbackable<StableAutoPolymorphicSmartPtrBackwardArchiver<T>>{

        private:

            std::shared_ptr<BackwardResolverControllable<T>> backward_resolver;

        public:

            StableAutoPolymorphicSmartPtrBackwardArchiver(std::shared_ptr<BackwardResolverControllable<T>> backward_resolver){

                this->backward_resolver = backward_resolver;

            }

            template <class T1, class T2>
            void put(Archivable<T1>& obj_archive, T2&& val){

                using stripped_type_ref = dgstd::remove_reference<T2>::type&;

                this->put_helper(obj_archive, reinterpret_cast<stripped_type_ref>(val));

            }

            template <class T1, class T2, class T3>
            void callback(T1 *, 
                          PolyClassIDType, 
                          Archivable<T2>& obj_archive, 
                          T3&& wrapper){
                
                T1 * deserialized_ptr = nullptr; 

                StablePtrWrapper<T1, NewDeletePolicy> stable_wrapper(deserialized_ptr);  
                obj_archive.put(obj_archive, stable_wrapper);

                StableAutoPolySmartPtrUtility::assign_ptr(std::forward<T3>(wrapper), deserialized_ptr);

            }

            template <class T1>
            constexpr bool is_archivable() const{

                return StableAutoPolySmartPtrUtility::is_auto_poly_smart_ptr<T1>();

            }
        
        private:

            template <class T1, class T2, class ...Args>
            void put_helper(Archivable<T1>& obj_archive, AutoPolymorphicSharedPtrWrapper<T2, PolyRegisteredClasses<Args...>>& wrapper){
                
                constexpr auto classes = PolyRegisteredClasses<Args...>();
                PolyClassIDType id{};
                T2 * ptr = nullptr; 

                obj_archive.put(obj_archive, id);

                this->backward_resolver->id_resolve(ptr, id, *this, classes, obj_archive, wrapper);

            }
        
            template <class T1, class T2, class ...Args>
            void put_helper(Archivable<T1>& obj_archive, AutoPolymorphicUniquePtrWrapper<T2, PolyRegisteredClasses<Args...>>& wrapper){

                constexpr auto classes = PolyRegisteredClasses<Args...>();
                PolyClassIDType id{};
                T2 * ptr = nullptr; 

                obj_archive.put(obj_archive, id);

                this->backward_resolver->id_resolve(ptr, id, *this, classes, obj_archive, wrapper);

            }

    };

};

#endif 