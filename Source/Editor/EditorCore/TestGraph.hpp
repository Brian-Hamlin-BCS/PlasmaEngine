#pragma once

namespace Plasma
{
    class TestGraphProperties : public Composite
    {
    public:
        typedef TestGraphProperties LightningSelf;

        TestGraphProperties(Composite* parent);
        ~TestGraphProperties();

    };

    class TestNodeGraph : public Composite
    {
    public:
        typedef TestNodeGraph LightningSelf;

        TestNodeGraph(Composite* parent);
        ~TestNodeGraph();

    private:
        PropertyView* mProperties;
    };
}