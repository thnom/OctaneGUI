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

#include "../Json.h"
#include "../Paint.h"
#include "../Theme.h"
#include "Button.h"
#include "Text.h"

namespace OctaneUI
{

Button::Button(Window* InWindow)
	: Control(InWindow)
	, m_State(State::None)
{
	m_Text = std::make_shared<Text>(InWindow);
	m_Text->SetParent(this);
}

Button::~Button()
{
}

Button* Button::SetPosition(float X, float Y)
{
	return SetPosition(Vector2(X, Y));
}

Button* Button::SetPosition(const Vector2& Position)
{
	Control::SetPosition(Position);
	return this;
}

Button* Button::SetSize(float Width, float Height)
{
	return SetSize(Vector2(Width, Height));
}

Button* Button::SetSize(const Vector2& Size)
{
	Control::SetSize(Size);
	return this;
}

Button* Button::SetExpand(Expand InExpand)
{
	Control::SetExpand(InExpand);
	return this;
}

Button* Button::SetText(const char* InText)
{
	m_Text->SetText(InText);
	UpdateSize();
	return this;
}

const char* Button::GetText() const
{
	return m_Text->GetText();
}

Button* Button::SetOnPressed(OnEmptySignature Fn)
{
	m_OnPressed = Fn;
	return this;
}

void Button::OnPaint(Paint& Brush) const
{
	std::shared_ptr<Theme> TheTheme = GetTheme();

	Color BackgroundColor;
	switch (m_State)
	{
	case State::Hovered: BackgroundColor = TheTheme->GetColor(Theme::Colors::Button_Hovered); break;
	case State::Pressed: BackgroundColor = TheTheme->GetColor(Theme::Colors::Button_Pressed); break;
	default: BackgroundColor = TheTheme->GetColor(Theme::Colors::Button);
	}

	Brush.Rectangle(GetAbsoluteBounds(), BackgroundColor);
	m_Text->OnPaint(Brush);
}

void Button::Update()
{
	const Vector2 Size = GetSize() * 0.5f;
	const Vector2 TextSize = m_Text->GetSize() * 0.5f;
	m_Text->SetPosition(Size - TextSize);
}

void Button::OnLoad(const Json& Root)
{
	Control::OnLoad(Root);

	m_Text->OnLoad(Root["Text"]);
	UpdateSize();
}

bool Button::OnMousePressed(const Vector2& Position, Mouse::Button Button)
{
	if (m_State == State::Hovered)
	{
		m_State = State::Pressed;
		Invalidate();
		return true;
	}

	return false;
}

void Button::OnMouseReleased(const Vector2& Position, Mouse::Button Button)
{
	const bool Hovered = Contains(Position);

	if (m_State == State::Pressed && Hovered)
	{
		if (m_OnPressed)
		{
			m_OnPressed();
		}
	}

	m_State = Hovered ? State::Hovered : State::None;
	Invalidate();
}

void Button::OnMouseEnter()
{
	if (m_State != State::Pressed)
	{
		m_State = State::Hovered;
	}
	
	Invalidate();
}

void Button::OnMouseLeave()
{
	if (m_State != State::Pressed)
	{
		m_State = State::None;
	}

	Invalidate();
}

void Button::UpdateSize()
{
	Vector2 Padding = GetTheme()->GetConstant(Theme::Vector2Constants::Button_Padding);
	Vector2 Size = m_Text->GetSize() + Padding * 2.0f;
	SetSize(Size);
	Update();
}

}
