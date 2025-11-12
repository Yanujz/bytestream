#include <gtest/gtest.h>
#include <bytestream/core.hpp>
#include <bytestream/serialization.hpp>
#include <vector>
#include <array>

using namespace bytestream;

// ============================================================================
// Test Fixtures
// ============================================================================

class SerializationTest : public ::testing::Test {
protected:
    std::vector<uint8_t> buffer;

    void SetUp() override
    {
        buffer.resize(1024, 0);
    }
};

// ============================================================================
// POD Struct Tests
// ============================================================================

struct SimplePOD
{
    int32_t  x;
    float    y;
    uint16_t z;
};

TEST_F(SerializationTest, PODStruct)
{
    SimplePOD p1{ 42, 3.14f, 100 };

    Writer w(buffer.data(), buffer.size());

    write_field(w, p1);

    Reader    r(buffer.data(), buffer.size());
    SimplePOD p2 = read_field<SimplePOD>(r);

    EXPECT_EQ(p2.x, 42);
    EXPECT_FLOAT_EQ(p2.y, 3.14f);
    EXPECT_EQ(p2.z, 100);
}

// ============================================================================
// Custom Serializable Tests
// ============================================================================

struct Person
{
    std::string name;
    uint32_t    age;
    float       height;

    void serialize(Writer &w) const
    {
        write_fields(w, name, age, height);
    }

    static Person deserialize(Reader &r)
    {
        Person p;
        p.name   = read_field<std::string>(r);
        p.age    = read_field<uint32_t>(r);
        p.height = read_field<float>(r);
        return p;
    }
};

TEST_F(SerializationTest, CustomSerializable)
{
    Person p1{ "Alice", 30, 1.65f };

    Writer w(buffer.data(), buffer.size());

    write_field(w, p1);

    Reader r(buffer.data(), buffer.size());
    Person p2 = read_field<Person>(r);

    EXPECT_EQ(p2.name, "Alice");
    EXPECT_EQ(p2.age, 30);
    EXPECT_FLOAT_EQ(p2.height, 1.65f);
}

// ============================================================================
// Nested Struct Tests
// ============================================================================

struct Vec2
{
    float x, y;

    void serialize(Writer &w) const
    {
        write_fields(w, x, y);
    }

    static Vec2 deserialize(Reader &r)
    {
        Vec2 v;
        v.x = read_field<float>(r);
        v.y = read_field<float>(r);
        return v;
    }

    bool operator==(const Vec2 &other) const
    {
        return x == other.x && y == other.y;
    }
};

struct Circle
{
    Vec2  center;
    float radius;

    void serialize(Writer &w) const
    {
        write_fields(w, center, radius);
    }

    static Circle deserialize(Reader &r)
    {
        Circle c;
        c.center = read_field<Vec2>(r);
        c.radius = read_field<float>(r);
        return c;
    }
};

TEST_F(SerializationTest, NestedStructs)
{
    Circle c1{ { 10.0f, 20.0f }, 5.0f };

    Writer w(buffer.data(), buffer.size());

    write_field(w, c1);

    Reader r(buffer.data(), buffer.size());
    Circle c2 = read_field<Circle>(r);

    EXPECT_EQ(c2.center, c1.center);
    EXPECT_FLOAT_EQ(c2.radius, 5.0f);
}

// ============================================================================
// Vector Tests
// ============================================================================

struct Score
{
    uint32_t player_id;
    uint32_t score;

    void serialize(Writer &w) const
    {
        write_fields(w, player_id, score);
    }

    static Score deserialize(Reader &r)
    {
        Score s;
        s.player_id = read_field<uint32_t>(r);
        s.score     = read_field<uint32_t>(r);
        return s;
    }
};

TEST_F(SerializationTest, VectorOfStructs)
{
    std::vector<Score> scores1 = {
        { 1, 100 },
        { 2, 200 },
        { 3, 300 }
    };

    Writer w(buffer.data(), buffer.size());

    write_vector(w, scores1);

    Reader             r(buffer.data(), buffer.size());
    std::vector<Score> scores2 = read_vector<Score>(r);

    ASSERT_EQ(scores2.size(), 3);
    EXPECT_EQ(scores2[0].player_id, 1);
    EXPECT_EQ(scores2[0].score, 100);
    EXPECT_EQ(scores2[1].player_id, 2);
    EXPECT_EQ(scores2[1].score, 200);
    EXPECT_EQ(scores2[2].player_id, 3);
    EXPECT_EQ(scores2[2].score, 300);
}

TEST_F(SerializationTest, VectorOfPrimitives)
{
    std::vector<int32_t> vec1 = { 10, 20, 30, 40, 50 };

    Writer w(buffer.data(), buffer.size());

    write_vector(w, vec1);

    Reader r(buffer.data(), buffer.size());
    std::vector<int32_t> vec2 = read_vector<int32_t>(r);

    EXPECT_EQ(vec2, vec1);
}

TEST_F(SerializationTest, EmptyVector)
{
    std::vector<uint32_t> vec1;

    Writer w(buffer.data(), buffer.size());

    write_vector(w, vec1);

    Reader r(buffer.data(), buffer.size());
    std::vector<uint32_t> vec2 = read_vector<uint32_t>(r);

    EXPECT_TRUE(vec2.empty());
}

// ============================================================================
// Array Tests
// ============================================================================

TEST_F(SerializationTest, FixedArray)
{
    std::array<float, 4> arr1 = { { 1.0f, 2.0f, 3.0f, 4.0f } };

    Writer w(buffer.data(), buffer.size());

    write_array(w, arr1);

    Reader r(buffer.data(), buffer.size());
    std::array<float, 4> arr2 = read_array<float, 4>(r);

    EXPECT_EQ(arr2, arr1);
}

// ============================================================================
// String Tests
// ============================================================================

TEST_F(SerializationTest, EmptyString)
{
    std::string str1 = "";

    Writer w(buffer.data(), buffer.size());

    write_field(w, str1);

    Reader      r(buffer.data(), buffer.size());
    std::string str2 = read_field<std::string>(r);

    EXPECT_EQ(str2, "");
}

TEST_F(SerializationTest, LongString)
{
    std::string str1(1000, 'A');

    buffer.resize(2048);
    Writer w(buffer.data(), buffer.size());
    write_field(w, str1);

    Reader      r(buffer.data(), buffer.size());
    std::string str2 = read_field<std::string>(r);

    EXPECT_EQ(str2, str1);
}

// ============================================================================
// CRTP Base Class Tests
// ============================================================================

struct Vehicle : public Serializable<Vehicle>
{
    std::string model;
    uint32_t    year;
    // constructors
    Vehicle() = default;
    Vehicle(std::string_view m, uint32_t y) : model(m), year(y)
    {
    }

    void serialize_impl(Writer &w) const
    {
        write_fields(w, model, year);
    }

    void deserialize_impl(Reader &r)
    {
        model = read_field<std::string>(r);
        year  = read_field<uint32_t>(r);
    }
};

TEST_F(SerializationTest, CRTPBase)
{
    Vehicle v1{ "Tesla Model 3", 2023 };

    Writer w(buffer.data(), buffer.size());

    v1.serialize(w);

    Reader  r(buffer.data(), buffer.size());
    Vehicle v2 = Vehicle::deserialize(r);

    EXPECT_EQ(v2.model, "Tesla Model 3");
    EXPECT_EQ(v2.year, 2023);
}

// ============================================================================
// Endianness Tests
// ============================================================================

struct EndiannessTest
{
    uint32_t value;

    void serialize(Writer &w) const
    {
        write_field_le(w, value);
    }

    static EndiannessTest deserialize(Reader &r)
    {
        EndiannessTest e;
        e.value = read_field_le<uint32_t>(r);
        return e;
    }
};

TEST_F(SerializationTest, LittleEndianField)
{
    EndiannessTest e1{ 0x12345678 };

    Writer w(buffer.data(), buffer.size());

    write_field(w, e1);

    // Check byte order (little endian)
    EXPECT_EQ(buffer[0], 0x78);
    EXPECT_EQ(buffer[1], 0x56);
    EXPECT_EQ(buffer[2], 0x34);
    EXPECT_EQ(buffer[3], 0x12);

    Reader         r(buffer.data(), buffer.size());
    EndiannessTest e2 = read_field<EndiannessTest>(r);

    EXPECT_EQ(e2.value, 0x12345678);
}

// ============================================================================
// Complex Example Test
// ============================================================================

struct GameState
{
    uint32_t              level;
    std::string           player_name;
    Vec2                  player_pos;
    std::vector<uint32_t> inventory_ids;

    void serialize(Writer &w) const
    {
        write_field(w, level);
        write_field(w, player_name);
        write_field(w, player_pos);
        write_vector(w, inventory_ids);
    }

    static GameState deserialize(Reader &r)
    {
        GameState gs;
        gs.level         = read_field<uint32_t>(r);
        gs.player_name   = read_field<std::string>(r);
        gs.player_pos    = read_field<Vec2>(r);
        gs.inventory_ids = read_vector<uint32_t>(r);
        return gs;
    }
};

TEST_F(SerializationTest, ComplexStructure)
{
    GameState gs1{
        5,
        "Player1",
        { 100.0f, 200.0f },
        { 1001, 1002, 1003, 2001, 2002 }
    };

    Writer w(buffer.data(), buffer.size());

    write_field(w, gs1);
    size_t bytes_written = w.position();

    Reader    r(buffer.data(), buffer.size());
    GameState gs2 = read_field<GameState>(r);

    EXPECT_EQ(gs2.level, 5);
    EXPECT_EQ(gs2.player_name, "Player1");
    EXPECT_FLOAT_EQ(gs2.player_pos.x, 100.0f);
    EXPECT_FLOAT_EQ(gs2.player_pos.y, 200.0f);
    EXPECT_EQ(gs2.inventory_ids.size(), 5);
    EXPECT_EQ(gs2.inventory_ids[0], 1001);
    EXPECT_EQ(gs2.inventory_ids[4], 2002);

    EXPECT_EQ(r.position(), bytes_written);
}

// ============================================================================
// Type Trait Tests
// ============================================================================

TEST(TypeTraitTest, TriviallySerializable)
{
    EXPECT_TRUE(detail::is_trivially_serializable_v<SimplePOD>);
    EXPECT_TRUE(detail::is_trivially_serializable_v<int32_t>);
    EXPECT_FALSE(detail::is_trivially_serializable_v<std::string>);
    EXPECT_FALSE(detail::is_trivially_serializable_v<std::vector<int> >);
}

TEST(TypeTraitTest, CustomSerializable)
{
    EXPECT_TRUE(detail::is_serializable_v<Person>);
    EXPECT_TRUE(detail::is_serializable_v<Circle>);
    EXPECT_TRUE(detail::is_serializable_v<Vehicle>);
    EXPECT_FALSE(detail::is_serializable_v<int>);
    EXPECT_FALSE(detail::is_serializable_v<std::string>);
}

// ============================================================================
// Additional Tests for Edge Cases
// ============================================================================

TEST_F(SerializationTest, MultipleWrites)
{
    Person p1{ "Bob", 25, 1.75f };
    Person p2{ "Charlie", 35, 1.80f };

    Writer w(buffer.data(), buffer.size());

    write_field(w, p1);
    write_field(w, p2);

    Reader r(buffer.data(), buffer.size());
    Person p3 = read_field<Person>(r);
    Person p4 = read_field<Person>(r);

    EXPECT_EQ(p3.name, "Bob");
    EXPECT_EQ(p3.age, 25);
    EXPECT_EQ(p4.name, "Charlie");
    EXPECT_EQ(p4.age, 35);
}

TEST_F(SerializationTest, PODWithDifferentSizes)
{
    struct MixedPOD
    {
        uint8_t  a;
        uint16_t b;
        uint32_t c;
        uint64_t d;
    };

    MixedPOD m1{ 0x12, 0x3456, 0x789ABCDE, 0xFEDCBA9876543210ULL };

    Writer w(buffer.data(), buffer.size());

    write_field(w, m1);

    Reader   r(buffer.data(), buffer.size());
    MixedPOD m2 = read_field<MixedPOD>(r);

    EXPECT_EQ(m2.a, 0x12);
    EXPECT_EQ(m2.b, 0x3456);
    EXPECT_EQ(m2.c, 0x789ABCDE);
    EXPECT_EQ(m2.d, 0xFEDCBA9876543210ULL);
}

TEST_F(SerializationTest, NestedVectors)
{
    struct Container
    {
        std::vector<std::vector<uint32_t> > data;

        void serialize(Writer &w) const
        {
            w.write_le<uint32_t>(static_cast<uint32_t>(data.size()));
            for(const auto &vec : data)
            {
                write_vector(w, vec);
            }
        }

        static Container deserialize(Reader &r)
        {
            Container c;
            uint32_t  size = r.read_le<uint32_t>();
            c.data.reserve(size);
            for(uint32_t i = 0; i < size; ++i)
            {
                c.data.push_back(read_vector<uint32_t>(r));
            }
            return c;
        }
    };

    Container c1;

    c1.data = { { 1, 2, 3 }, { 4, 5 }, { 6, 7, 8, 9 } };

    Writer w(buffer.data(), buffer.size());
    write_field(w, c1);

    Reader    r(buffer.data(), buffer.size());
    Container c2 = read_field<Container>(r);

    ASSERT_EQ(c2.data.size(), 3);
    EXPECT_EQ(c2.data[0], std::vector<uint32_t>({ 1, 2, 3 }));
    EXPECT_EQ(c2.data[1], std::vector<uint32_t>({ 4, 5 }));
    EXPECT_EQ(c2.data[2], std::vector<uint32_t>({ 6, 7, 8, 9 }));
}
