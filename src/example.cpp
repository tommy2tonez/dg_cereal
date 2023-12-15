#include "serialization.h"
#include "assert.h"
#include <iostream>

struct Foo{

    size_t a;
    size_t b;
    std::vector<size_t> c;

    template <class Archive>
    void dg_reflect(const Archive& arch) const{
        arch(a,b,c);
    }

    template <class Archive>
    void dg_reflect(const Archive& arch){
        arch(a,b,c);
    }

    bool operator ==(const Foo& other) const{
        return std::tie(a,b,c) == std::tie(other.a, other.b, other.c);
    }
};

struct Bar{

    std::optional<size_t> a;
    std::unordered_map<size_t, size_t> b;
    Foo c;
    std::string d;

    template <class Archive>
    void dg_reflect(const Archive& arch) const{
        arch(a,b,c,d);
    }

    template <class Archive>
    void dg_reflect(const Archive& arch){
        arch(a,b,c,d);
    }

    bool operator ==(const Bar& other) const{
        return std::tie(a,b,c,d) == std::tie(other.a, other.b, other.c, other.d);
    }
};

int main(){

    //let U be the set of all involving types, serialization is defined when U c {arithmetics, std::unique_ptr<T>, std::optional, std tuple-like objects(tuple, pair, array),
    //std::vector, std::unordered_map, std::map, std::unordered_set, std::set, std alias of std::basic_string<Args...>, exclusive (*) reflectible} 
    //(*) is and only is reflectible

    Bar bar{std::nullopt, {{1, 2}, {2, 3}}, {1, 2, {2, 3}}, "b"};

    auto serialized     = dg::compact_serializer::serialize(bar);
    auto deserialized   = dg::compact_serializer::deserialize<Bar>(serialized.first.get(), serialized.second);

    assert(bar == deserialized);
}