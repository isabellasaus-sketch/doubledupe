// MTRX3760 2026 Project 2: Package Deal
// File: test_map_model.cpp
// Author(s): Isabella Armitstead
// Description: Unit tests for the map model (Room, Passage, TopologyGraph).

#include <gtest/gtest.h>

#include "mapper/Passage.hpp"
#include "mapper/Room.hpp"
#include "mapper/TopologyGraph.hpp"

TEST(RoomTest, StartsUntyped)
{
    Room room;
    EXPECT_FALSE(room.hasType());
    EXPECT_TRUE(room.type().empty());
}

TEST(RoomTest, SetTypeStoresLabel)
{
    Room room;
    room.setType("SMALL_SQUARE");
    EXPECT_TRUE(room.hasType());
    EXPECT_EQ(room.type(), "SMALL_SQUARE");
}

TEST(RoomTest, TypeIsSetOnceAndNeverOverwritten)
{
    Room room;
    room.setType("SMALL_SQUARE");
    room.setType("RECTANGLE");
    EXPECT_EQ(room.type(), "SMALL_SQUARE");
}

TEST(PassageTest, ReportsTagAndEndpoints)
{
    Passage passage(7, 1, 2);
    EXPECT_EQ(passage.tagId(), 7);
    EXPECT_TRUE(passage.connectsRoom(1));
    EXPECT_TRUE(passage.connectsRoom(2));
    EXPECT_FALSE(passage.connectsRoom(3));
}

TEST(PassageTest, ExposesBothEndpoints)
{
    Passage passage(7, 1, 2);
    EXPECT_EQ(passage.roomA(), 1u);
    EXPECT_EQ(passage.roomB(), 2u);
}

TEST(PassageTest, OtherEndReturnsOppositeRoom)
{
    Passage passage(7, 1, 2);
    EXPECT_EQ(passage.otherEnd(1), 2u);
    EXPECT_EQ(passage.otherEnd(2), 1u);
}

TEST(PassageTest, FullyCrossedAfterTwoCrossings)
{
    Passage passage(7, 0, 1);
    EXPECT_FALSE(passage.isFullyCrossed());
    passage.recordCrossing();
    EXPECT_FALSE(passage.isFullyCrossed());
    passage.recordCrossing();
    EXPECT_TRUE(passage.isFullyCrossed());
}

TEST(TopologyGraphTest, StartsWithOneRoomAndNoPassages)
{
    TopologyGraph graph;
    EXPECT_EQ(graph.statusText(), "Rooms: 1, passages: 0, current room: 1");
}

TEST(TopologyGraphTest, UnseenTagCreatesRoomAndPassage)
{
    TopologyGraph graph;
    EXPECT_TRUE(graph.crossPassage(4));
    EXPECT_EQ(graph.statusText(), "Rooms: 2, passages: 1, current room: 2");
}

TEST(TopologyGraphTest, KnownTagMovesBackWithoutNewRoom)
{
    TopologyGraph graph;
    graph.crossPassage(4);
    EXPECT_TRUE(graph.crossPassage(4));
    EXPECT_EQ(graph.statusText(), "Rooms: 2, passages: 1, current room: 1");
}

TEST(TopologyGraphTest, KnownTagFromWrongRoomResyncsThroughNearerEnd)
{
    TopologyGraph graph;
    graph.crossPassage(4);   // room 1 -> room 2
    graph.crossPassage(7);   // room 2 -> room 3
    // Tag 7 back into room 2 was missed. Tag 4 joins rooms 1 and 2, so the
    // robot must have reached room 2 and is now crossing into room 1.
    EXPECT_FALSE(graph.crossPassage(4));
    EXPECT_EQ(graph.statusText(), "Rooms: 3, passages: 2, current room: 1");
}

TEST(TopologyGraphTest, MissedCrossingIsCreditedSoTraversalCanFinish)
{
    TopologyGraph graph;
    graph.crossPassage(4);   // 1 -> 2
    graph.crossPassage(7);   // 2 -> 3
    graph.crossPassage(4);   // 3 -> 2 missed, then 2 -> 1
    EXPECT_TRUE(graph.isFullyTraversed());
}

TEST(TopologyGraphTest, ResyncAcrossSeveralMissedRooms)
{
    TopologyGraph graph;
    graph.crossPassage(4);   // 1 -> 2
    graph.crossPassage(7);   // 2 -> 3
    graph.crossPassage(9);   // 3 -> 4
    // Tags 9 and 7 on the way back were both missed.
    EXPECT_FALSE(graph.crossPassage(4));
    EXPECT_EQ(graph.statusText(), "Rooms: 4, passages: 3, current room: 1");
    EXPECT_TRUE(graph.isFullyTraversed());
}

TEST(TopologyGraphTest, MapStaysUsableAfterResync)
{
    TopologyGraph graph;
    graph.crossPassage(4);   // 1 -> 2
    graph.crossPassage(7);   // 2 -> 3
    graph.crossPassage(4);   // resync: 3 -> 2 missed, then 2 -> 1
    // Later tags are accepted normally instead of being rejected forever.
    EXPECT_TRUE(graph.crossPassage(4));   // 1 -> 2
    EXPECT_TRUE(graph.crossPassage(12));  // 2 -> new room 4
    EXPECT_EQ(graph.statusText(), "Rooms: 4, passages: 3, current room: 4");
}

TEST(TopologyGraphTest, NotFullyTraversedBeforeAnyPassageIsCrossed)
{
    // The robot may see the start tile at launch, before it has moved.
    TopologyGraph graph;
    EXPECT_FALSE(graph.isFullyTraversed());
}

TEST(TopologyGraphTest, FullyTraversedOnlyWhenEveryPassageCrossedBothWays)
{
    TopologyGraph graph;
    graph.crossPassage(4);   // 1 -> 2
    graph.crossPassage(7);   // 2 -> 3
    EXPECT_FALSE(graph.isFullyTraversed());
    graph.crossPassage(7);   // 3 -> 2
    EXPECT_FALSE(graph.isFullyTraversed());
    graph.crossPassage(4);   // 2 -> 1
    EXPECT_TRUE(graph.isFullyTraversed());
}

TEST(TopologyGraphDotTest, EmptyMapHasOneUnknownRoom)
{
    TopologyGraph graph;
    EXPECT_EQ(graph.toDot(),
              "@startdot\n"
              "graph arena {\n"
              "  r1 [label=\"unknown\"];\n"
              "}\n"
              "@enddot\n");
}

TEST(TopologyGraphDotTest, ListsRoomsThenPassagesWithTagIds)
{
    TopologyGraph graph;
    graph.crossPassage(4);   // r1 -- r2
    graph.crossPassage(7);   // r2 -- r3
    graph.crossPassage(7);   // back to r2, no new edge
    graph.crossPassage(9);   // r2 -- r4
    EXPECT_EQ(graph.toDot(),
              "@startdot\n"
              "graph arena {\n"
              "  r1 [label=\"unknown\"];\n"
              "  r2 [label=\"unknown\"];\n"
              "  r3 [label=\"unknown\"];\n"
              "  r4 [label=\"unknown\"];\n"
              "  r1 -- r2 [label=\"tag 4\"];\n"
              "  r2 -- r3 [label=\"tag 7\"];\n"
              "  r2 -- r4 [label=\"tag 9\"];\n"
              "}\n"
              "@enddot\n");
}

class TopologyGraphLabelTest : public ::testing::Test
{
protected:
    // Counts how many rooms in the DOT text carry this exact label.
    static std::size_t countLabel(const std::string& dot, const std::string& label)
    {
        const std::string needle = "[label=\"" + label + "\"];";
        std::size_t count = 0;
        for (std::size_t at = dot.find(needle); at != std::string::npos;
             at = dot.find(needle, at + 1)) {
            ++count;
        }
        return count;
    }
};

TEST_F(TopologyGraphLabelTest, FirstRoomTakesFirstLabel)
{
    TopologyGraph graph;
    graph.assignTypeLabel("SMALL_SQUARE");
    EXPECT_EQ(countLabel(graph.toDot(), "small square"), 1u);
}

TEST_F(TopologyGraphLabelTest, LabelArrivingAfterCrossingTypesNewRoom)
{
    TopologyGraph graph;
    graph.assignTypeLabel("SMALL_SQUARE");
    graph.crossPassage(4);
    graph.assignTypeLabel("RECTANGLE");
    const std::string dot = graph.toDot();
    EXPECT_EQ(countLabel(dot, "small square"), 1u);
    EXPECT_EQ(countLabel(dot, "rectangle"), 1u);
}

TEST_F(TopologyGraphLabelTest, LabelArrivingBeforeCrossingTypesNewRoom)
{
    TopologyGraph graph;
    graph.assignTypeLabel("SMALL_SQUARE");
    graph.assignTypeLabel("RECTANGLE");   // early: room 1 is already typed
    graph.crossPassage(4);                // room 2 picks up the stored label
    const std::string dot = graph.toDot();
    EXPECT_EQ(countLabel(dot, "small square"), 1u);
    EXPECT_EQ(countLabel(dot, "rectangle"), 1u);
}

TEST_F(TopologyGraphLabelTest, StaleLabelOfPreviousRoomIsNotUsed)
{
    TopologyGraph graph;
    graph.assignTypeLabel("SMALL_SQUARE");
    graph.crossPassage(4);   // room 2: latest label is still room 1's
    EXPECT_EQ(countLabel(graph.toDot(), "small square"), 1u);
    EXPECT_EQ(countLabel(graph.toDot(), "unknown"), 1u);
}

TEST_F(TopologyGraphLabelTest, TypeIsNeverOverwritten)
{
    TopologyGraph graph;
    graph.assignTypeLabel("SMALL_SQUARE");
    graph.assignTypeLabel("RECTANGLE");   // still in room 1
    EXPECT_EQ(countLabel(graph.toDot(), "small square"), 1u);
    EXPECT_EQ(countLabel(graph.toDot(), "rectangle"), 0u);
}

TEST_F(TopologyGraphLabelTest, ReturningToTypedRoomKeepsItsType)
{
    TopologyGraph graph;
    graph.assignTypeLabel("SMALL_SQUARE");
    graph.crossPassage(4);
    graph.assignTypeLabel("RECTANGLE");
    graph.crossPassage(4);                // back to room 1
    graph.assignTypeLabel("SMALL_SQUARE");
    const std::string dot = graph.toDot();
    EXPECT_EQ(countLabel(dot, "small square"), 1u);
    EXPECT_EQ(countLabel(dot, "rectangle"), 1u);
}

TEST(TopologyGraphDotTest, ReproducesHandoutExampleStructure)
{
    TopologyGraph graph;
    graph.assignTypeLabel("SMALL_SQUARE");
    graph.crossPassage(4);                // r1 -> r2
    graph.assignTypeLabel("RECTANGLE");
    graph.crossPassage(7);                // r2 -> r3
    graph.assignTypeLabel("TRIANGLE");
    graph.crossPassage(7);                // back to r2
    graph.assignTypeLabel("RECTANGLE");   // room shape republishes on return
    graph.crossPassage(9);                // r2 -> r4
    graph.assignTypeLabel("LARGE_SQUARE");
    EXPECT_EQ(graph.toDot(),
              "@startdot\n"
              "graph arena {\n"
              "  r1 [label=\"small square\"];\n"
              "  r2 [label=\"rectangle\"];\n"
              "  r3 [label=\"triangle\"];\n"
              "  r4 [label=\"large square\"];\n"
              "  r1 -- r2 [label=\"tag 4\"];\n"
              "  r2 -- r3 [label=\"tag 7\"];\n"
              "  r2 -- r4 [label=\"tag 9\"];\n"
              "}\n"
              "@enddot\n");
}