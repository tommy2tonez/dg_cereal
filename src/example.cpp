#include "serialization.h"
#include "assert.h"

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

    template <class Archive>
    void dg_reflect(const Archive& arch) const{
        arch(a,b,c);
    }

    template <class Archive>
    void dg_reflect(const Archive& arch){
        arch(a,b,c);
    }

    bool operator ==(const Bar& other) const{
        return std::tie(a,b,c) == std::tie(other.a, other.b, other.c);
    }
};

int main(){

    Bar bar{std::nullopt, {{1, 2}, {2, 3}}, {1, 2, {2, 3}}};

    auto serialized     = dg::compact_serializer::serialize(bar);
    auto deserialized   = dg::compact_serializer::deserialize<Bar>(serialized.first.get(), serialized.second);

    assert(bar == deserialized);
}