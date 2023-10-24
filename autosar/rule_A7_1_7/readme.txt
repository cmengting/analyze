void F() noexcept
{
    std::int32_t x{5};
    std::int32_t y{15};  //Non-compliant
    x++;
    ++y; //Non-compliant
}

The above is a bad case mentioned in the book, while we think it's a good case.
