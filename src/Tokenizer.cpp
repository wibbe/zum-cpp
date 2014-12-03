
#include "Tokenizer.h"

Tokenizer::Tokenizer(Str const& stream)
  : stream_(stream),
    pos_(0),
    value_()
{ }

Token Tokenizer::next()
{
  value_.clear();
  eatWhitespace();

  if (eof())
    return Token::EndOfFile;

  if (isDigit(current()))
  {
    return parseNumber();
  }
  else if (current() == '-')
  {
    if (isDigit(peak()))
    {
      // Eat - and then parse the number
      value_.append('-');
      return parseNumber();
    }
    else
    {
      return parseIdentifier();
    }
  }
  else if (current() == '(')
  {
    step();
    return Token::LeftParenthesis;
  }
  else if (current() == ')')
  {
    step();
    return Token::RightParenthesis;
  }
  else if (isOperator(current()) && (isWhitespace(peak()) || isAlpha(peak()) || isDigit(peak())))
  {
    value_.append(current());
    step();

    return Token::Operator;
  }
  else if (isAlpha(current()))
  {
    return parseIdentifier();
  }

  // If we get here, we have encountered an error
  value_.append(Str("Parse error - unknown character: "))
        .append(current());

  return Token::Error;
}

void Tokenizer::eatWhitespace()
{
  while (!eof() && isWhitespace(current()))
    step();
}

Token Tokenizer::parseNumber()
{
  value_.append(current());
  step();

  while (!eof() && isDigit(current()))
  {
    value_.append(current());
    step();
  }

  if (current() == '.')
  {
    if (!isDigit(peak()))
    {
      const Str number = value_;
      value_.set("Parse error - expected digit but got ");
      value_.append(peak())
            .append(Str(" in number "))
            .append(number);
      Token::Error;
    }
    else
    {
      value_.append(current());
      step();

      while (!eof() && isDigit(current()))
      {
        value_.append(current());
        step();
      }
    }
  }

  return Token::Number;
}

Token Tokenizer::parseIdentifier()
{
  Token type = isUpperAlpha(current()) ? Token::Cell : Token::Identifier;
  bool hasNumber = false;

  value_.append(current());
  step();

  while (!eof())
  {
    if (type == Token::Cell)
    {

    }

    if (isAlpha(current()))


    value_.append(current());


    step();
  }


  return type;
}
