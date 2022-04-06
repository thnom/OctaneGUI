/**

MIT License

Copyright (c) 2022 Mitchell Davis <mdavisprog@gmail.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

#include "ListBox.h"
#include "../Json.h"
#include "../Paint.h"
#include "../ThemeProperties.h"
#include "Panel.h"
#include "ScrollableContainer.h"
#include "VerticalContainer.h"

namespace OctaneGUI
{

class ListBoxInteraction : public Control
{
	CLASS(ListBoxInteraction)

public:
	typedef std::function<void(int, int)> OnChangeSignature;

	ListBoxInteraction(Window* InWindow, const std::shared_ptr<ScrollableContainer>& Scrollable, const std::shared_ptr<Container>& List)
		: Control(InWindow)
		, m_Scrollable(Scrollable)
		, m_List(List)
	{
		SetExpand(Expand::Both);
	}

	int Index() const
	{
		return m_Index;
	}

	int HoveredIndex() const
	{
		return m_Hovered_Index;
	}

	ListBoxInteraction& SetOnSelect(ListBox::OnSelectSignature Fn)
	{
		m_OnSelect = Fn;
		return *this;
	}

	ListBoxInteraction& SetOnHoverChange(OnChangeSignature Fn)
	{
		m_OnHoverChange = Fn;
		return *this;
	}

	ListBoxInteraction& SetOnSelectionChange(OnChangeSignature Fn)
	{
		m_OnSelectionChange = Fn;
		return *this;
	}

	virtual void OnMouseMove(const Vector2& Position) override
	{
		if (m_Scrollable.expired() || m_List.expired())
		{
			return;
		}

		std::shared_ptr<ScrollableContainer> Scrollable = m_Scrollable.lock();
		std::shared_ptr<Container> List = m_List.lock();

		Scrollable->OnMouseMove(Position);
		if (Scrollable->IsInScrollBar(Position))
		{
			SetHoveredIndex(-1);
			return;
		}

		int Index = 0;
		for (const std::shared_ptr<Control>& ListItem : List->Controls())
		{
			const Vector2 ItemPos = ListItem->GetAbsolutePosition();
			const Rect Bounds = { ItemPos, ItemPos + Vector2(Scrollable->GetSize().X, ListItem->GetSize().Y) };

			if (Bounds.Contains(Position + Vector2(Scrollable->GetPosition().X, 0.0f)))
			{
				break;
			}

			Index++;
		}

		int NewIndex = -1;
		if (Index < List->Controls().size())
		{
			NewIndex = Index;
		}
		else
		{
			NewIndex = -1;
		}

		SetHoveredIndex(NewIndex);
	}

	virtual bool OnMousePressed(const Vector2& Position, Mouse::Button Button) override
	{
		if (!m_Scrollable.expired())
		{
			std::shared_ptr<ScrollableContainer> Scrollable = m_Scrollable.lock();
			if (Scrollable->OnMousePressed(Position, Button))
			{
				return true;
			}
		}

		if (m_Hovered_Index == -1)
		{
			return false;
		}

		if (Button == Mouse::Button::Left)
		{
			if (m_OnSelectionChange)
			{
				m_OnSelectionChange(m_Hovered_Index, m_Index);
			}

			m_Index = m_Hovered_Index;
			if (m_OnSelect && !m_List.expired())
			{
				m_OnSelect(m_Index, m_List.lock()->Controls()[m_Index]);
			}
			Invalidate();
		}

		return false;
	}

	virtual void OnMouseReleased(const Vector2& Position, Mouse::Button Button) override
	{
		if (!m_Scrollable.expired())
		{
			m_Scrollable.lock()->OnMouseReleased(Position, Button);
		}
	}

	virtual void OnMouseLeave() override
	{
		SetHoveredIndex(-1);
	}

private:
	void SetHoveredIndex(int Index)
	{
		if (m_Hovered_Index != Index)
		{
			if (m_OnHoverChange)
			{
				m_OnHoverChange(Index, m_Hovered_Index);
			}
			m_Hovered_Index = Index;
			Invalidate();
		}
	}

	std::weak_ptr<ScrollableContainer> m_Scrollable {};
	std::weak_ptr<Container> m_List {};
	int m_Index { -1 };
	int m_Hovered_Index { -1 };
	ListBox::OnSelectSignature m_OnSelect { nullptr };
	OnChangeSignature m_OnHoverChange { nullptr };
	OnChangeSignature m_OnSelectionChange { nullptr };
};

ListBox::ListBox(Window* InWindow)
	: ScrollableViewControl(InWindow)
{
	m_Panel = AddControl<Panel>();
	m_Panel->SetExpand(Expand::Both);

	m_List = Scrollable()->AddControl<VerticalContainer>();
	m_List->SetSpacing({ 0.0f, 0.0f });

	m_Interaction = AddControl<ListBoxInteraction>(Scrollable(), m_List);
	m_Interaction
		->SetOnSelect([this](int Index, std::weak_ptr<Control> Item) -> void
			{
				if (m_OnSelect)
				{
					m_OnSelect(Index, Item);
				}
			})
		.SetOnHoverChange([this](int New, int Old) -> void
			{
				if (New != -1)
				{
					m_List->Controls()[New]->SetProperty(ThemeProperties::Text, GetProperty(ThemeProperties::TextSelectable_Text_Hovered).ToColor());
				}

				if (Old != -1 && Old != m_Interaction->Index())
				{
					m_List->Controls()[Old]->ClearProperty(ThemeProperties::Text);
				}
			})
		.SetOnSelectionChange([this](int New, int Old) -> void
			{
				if (Old != -1 && New != Old)
				{
					m_List->Controls()[Old]->ClearProperty(ThemeProperties::Text);
				}
			});

	SetSize({ 200.0f, 200.0f });
}

int ListBox::Index() const
{
	return m_Interaction->Index();
}

ListBox* ListBox::SetOnSelect(OnSelectSignature Fn)
{
	m_OnSelect = Fn;
	return this;
}

Vector2 ListBox::ListSize() const
{
	return m_List->DesiredSize();
}

void ListBox::OnLoad(const Json& Root)
{
	Json Copy = Root;
	Copy["Controls"] = Json();

	Container::OnLoad(Copy);

	Json List(Json::Type::Object);
	List["Controls"] = Root["Controls"];
	m_List->OnLoad(List);
}

void ListBox::OnPaint(Paint& Brush) const
{
	m_Panel->OnPaint(Brush);

	Brush.PushClip(GetAbsoluteBounds());

	const int HoveredIndex = m_Interaction->HoveredIndex();
	const int Index = m_Interaction->Index();

	if (HoveredIndex != -1 && HoveredIndex != Index)
	{
		const std::shared_ptr<Control>& Item = m_List->Controls()[HoveredIndex];
		PaintItem(Brush, Item);
	}

	if (Index != -1)
	{
		const std::shared_ptr<Control>& Item = m_List->Controls()[Index];
		PaintItem(Brush, Item);
	}

	Brush.PopClip();

	Scrollable()->OnPaint(Brush);
}

void ListBox::InsertItem(const std::shared_ptr<Control>& Item)
{
	m_List->InsertControl(Item);
}

void ListBox::PaintItem(Paint& Brush, const std::shared_ptr<Control>& Item) const
{
	float ContentWidth = std::max<float>(Scrollable()->ContentSize().X, GetSize().X);
	Rect Bounds = Item->GetAbsoluteBounds();
	if (Bounds.GetSize().X < ContentWidth)
	{
		Bounds.SetSize({ ContentWidth, Bounds.Height() });
	}
	Brush.Rectangle(Bounds, GetProperty(ThemeProperties::TextSelectable_Hovered).ToColor());
}

}