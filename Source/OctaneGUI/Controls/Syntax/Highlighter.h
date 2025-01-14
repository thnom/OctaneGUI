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

#pragma once

#include <memory>
#include <string>
#include <vector>

namespace OctaneGUI
{

struct Color;
class Text;
class TextInput;
struct TextSpan;

namespace Syntax
{

class Highlighter
{
public:
    Highlighter();

    Highlighter& SetInput(const std::weak_ptr<TextInput>& Input);
    Color DefaultColor() const;
    std::vector<TextSpan> GetSpans(const std::u32string_view& Span) const;

protected:
    virtual std::vector<TextSpan> Update(const std::u32string_view& Span) const = 0;

private:
    std::weak_ptr<TextInput> m_Input {};
};

}
}
