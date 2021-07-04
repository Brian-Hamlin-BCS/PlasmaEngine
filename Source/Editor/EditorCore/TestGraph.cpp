#include "Precompiled.hpp"

namespace Plasma
{
    TestGraphProperties::TestGraphProperties(Composite* parent) : Composite(parent)
    {

    }

    TestGraphProperties::~TestGraphProperties()
    {

    }

    TestNodeGraph::TestNodeGraph(Composite* parent) : Composite(parent)
    {
        this->SetLayout(CreateStackLayout(LayoutDirection::TopToBottom, Vec2::cZero, Thickness::cZero));

        Composite* top = new Composite(this);
        top->SetLayout(CreateRowLayout());
        top->SetSizing(SizeAxis::Y, SizePolicy::Flex, 20);

        Composite* bottom = new Composite(this);
        bottom->SetLayout(CreateStackLayout(LayoutDirection::RightToLeft, Pixels(4, 4), Thickness(4, 4, 4, 4)));

        Composite* left = new Composite(top);
        left->SetSizing(SizeAxis::X, SizePolicy::Flex, 35);
        left->SetLayout(CreateStackLayout());

        mProperties = new PropertyView(left);
        mProperties->SetSizing(SizeAxis::Y, SizePolicy::Flex, 30);
        mProperties->ActivateAutoUpdate();

        Splitter* splitter = new Splitter(top);
        splitter->SetSize(Pixels(2, 2));

        Composite* right = new Composite(top);
        right->SetSizing(SizeAxis::X, SizePolicy::Flex, 65);
        right->SetLayout(CreateStackLayout());

        Composite* buttonRow = new Composite(right);
        buttonRow->SetLayout(CreateStackLayout(LayoutDirection::LeftToRight, Vec2::cZero, Thickness::cZero));
    }

    TestNodeGraph::~TestNodeGraph()
    {

    }
}