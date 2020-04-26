#include <iostream>
#include <type_traits>
#include <limits>

#if defined(min) || defined(max)
#   error min() and max() macros must not be defined. For Windows, define NOMINMAX before including the Windows headers.
#endif

namespace integral_io
{
    // Generic trait which handles any signed or unsigned integer which is bigger than 1 byte.
    template <typename Integer, typename = std::enable_if_t<std::is_integral_v<Integer>, Integer>, std::size_t = sizeof(Integer), bool = std::is_signed<Integer>::value>
    struct integral_io_trait
    {
        using type = Integer;
    };

    // Trait specialised for signed integers which are exactly 1 byte.
    template <typename Integer>
    struct integral_io_trait<Integer, Integer, 1, true>
    {
        using type = std::int16_t;
    };

    // Trait specialised for unsigned integers which are exactly 1 byte.
    template <typename Integer>
    struct integral_io_trait<Integer, Integer, 1, false>
    {
        using type = std::uint16_t;
    };

    // Helper template alias.
    template <typename Integer>
    using integral_io_t = typename integral_io_trait<Integer>::type;

    // Generic output-only wrapper for signed and unsigned integers which are bigger than 1 byte.
    template <typename Integer, typename = typename std::enable_if<std::is_integral<Integer>::value, Integer>::type, std::size_t = sizeof(Integer)>
    struct integral_output_wrapper final
    {
        integral_output_wrapper(const Integer value) : m_value{ value } {}
        integral_output_wrapper(integral_output_wrapper&) = default;
        integral_output_wrapper(integral_output_wrapper&&) = default;
        integral_output_wrapper& operator=(const integral_output_wrapper&) = delete;
        integral_output_wrapper& operator=(integral_output_wrapper&&) = delete;
        ~integral_output_wrapper() = default;

        template <typename Elem, typename Traits>
        void output(std::basic_ostream<Elem, Traits>& os) const
        {
            os << m_value;
        }

        const Integer m_value;
    };

    // Output-only wrapper specialised for signed and unsigned integers which are exactly 1 byte.
    template <typename Integer>
    struct integral_output_wrapper<Integer, Integer, 1> final
    {
        integral_output_wrapper(const Integer value) : m_value{ value } {}
        integral_output_wrapper(integral_output_wrapper&) = default;
        integral_output_wrapper(integral_output_wrapper&&) = default;
        integral_output_wrapper& operator=(const integral_output_wrapper&) = delete;
        integral_output_wrapper& operator=(integral_output_wrapper&&) = delete;
        ~integral_output_wrapper() = default;

        template <typename Elem, typename Traits>
        void output(std::basic_ostream<Elem, Traits>& os) const
        {
            os << static_cast<integral_io_t<Integer>>(m_value);
        }

        const Integer m_value;
    };

    // Generic input/output wrapper for signed and unsigned integers which are bigger than 1 byte.
    template <typename Integer, typename = typename std::enable_if<std::is_integral<Integer>::value, Integer>::type, std::size_t = sizeof(Integer), bool = std::is_signed<Integer>::value>
    struct integral_io_wrapper
    {
        integral_io_wrapper(Integer& value) : m_value{ value } {}
        integral_io_wrapper(integral_io_wrapper&) = default;
        integral_io_wrapper(integral_io_wrapper&&) = default;
        integral_io_wrapper& operator=(const integral_io_wrapper&) = delete;
        integral_io_wrapper& operator=(integral_io_wrapper&&) = delete;
        ~integral_io_wrapper() = default;

        template <typename Elem, typename Traits>
        void output(std::basic_ostream<Elem, Traits>& os) const
        {
            os << m_value;
        }

        template <typename Elem, typename Traits>
        void input(std::basic_istream<Elem, Traits>& is)
        {
            is >> m_value;
        }

        Integer& m_value;
    };

    // Input/output wrapper specialised for signed integers which are exactly 1 byte.
    template <typename Integer>
    struct integral_io_wrapper<Integer, Integer, 1, true>
    {
        integral_io_wrapper(Integer& value) : m_value{ value } {}
        integral_io_wrapper(integral_io_wrapper&) = default;
        integral_io_wrapper(integral_io_wrapper&&) = default;
        integral_io_wrapper& operator=(const integral_io_wrapper&) = delete;
        integral_io_wrapper& operator=(integral_io_wrapper&&) = delete;
        ~integral_io_wrapper() = default;

        template <typename Elem, typename Traits>
        void output(std::basic_ostream<Elem, Traits>& os) const
        {
            os << static_cast<std::int16_t>(m_value);
        }

        template <typename Elem, typename Traits>
        void input(std::basic_istream<Elem, Traits>& is)
        {
            std::int16_t temp;
            is >> temp;

            // Emulate the stream's usual bounds-checking behaviour for signed types.
            if (temp > static_cast<std::int16_t>(std::numeric_limits<Integer>::max()))
            {
                m_value = std::numeric_limits<Integer>::max();
                is.setstate(std::ios_base::failbit);
                return;
            }
            if (temp < static_cast<std::int16_t>(std::numeric_limits<Integer>::min()))
            {
                m_value = std::numeric_limits<Integer>::min();
                is.setstate(std::ios_base::failbit);
                return;
            }

            m_value = static_cast<Integer>(temp);
        }

        Integer& m_value;
    };

    // Input/output wrapper specialised for unsigned integers which are exactly 1 byte.
    template <typename Integer>
    struct integral_io_wrapper<Integer, Integer, 1, false>
    {
        integral_io_wrapper(Integer& value) : m_value{ value } {}
        integral_io_wrapper(integral_io_wrapper&) = default;
        integral_io_wrapper(integral_io_wrapper&&) = default;
        integral_io_wrapper& operator=(const integral_io_wrapper&) = delete;
        integral_io_wrapper& operator=(integral_io_wrapper&&) = delete;
        ~integral_io_wrapper() = default;

        template <typename Elem, typename Traits>
        void output(std::basic_ostream<Elem, Traits>& os) const
        {
            os << static_cast<std::int16_t>(m_value);
        }

        template <typename Elem, typename Traits>
        void input(std::basic_istream<Elem, Traits>& is)
        {
            // We have to do signed input so that we can correctly handle negatives which wrap around.
            // If we use unsigned then we won't be able to tell the difference between a positive value
            //  which is too big, and a negative value which has wrapped round.
            std::int16_t temp;
            is >> temp;

            // We need to emulate the stream's usual bounds-checking behaviour for signed types.
            if (temp > static_cast<std::int16_t>(std::numeric_limits<Integer>::max()))
            {
                m_value = std::numeric_limits<Integer>::max();
                is.setstate(std::ios_base::failbit);
                return;
            }
            if (temp < 0)
            {
                // A negative number is allowed to wrap around to positive, as long as its magnitude is
                //  less than the maximum representable value.
                if ((temp * -1) <= static_cast<std::int16_t>(std::numeric_limits<Integer>::max()))
                {
                    m_value = static_cast<Integer>(std::numeric_limits<Integer>::max() + temp + 1);
                    return;
                }

                // Any negative numbers with a larger magnitude are out of bounds.
                m_value = std::numeric_limits<Integer>::max();
                is.setstate(std::ios_base::failbit);
                return;
            }

            m_value = static_cast<Integer>(temp);
        }

        Integer& m_value;
    };

    // Stream operators:
    template <typename Elem, typename Traits, typename Integer>
    std::basic_ostream<Elem, Traits>& operator<<(std::basic_ostream<Elem, Traits>& os, const integral_output_wrapper<Integer>&& wrapper)
    {
        wrapper.output(os);
        return os;
    }

    template <typename Elem, typename Traits, typename Integer>
    std::basic_ostream<Elem, Traits>& operator<<(std::basic_ostream<Elem, Traits>& os, const integral_io_wrapper<Integer>&& wrapper)
    {
        wrapper.output(os);
        return os;
    }

    template <typename Elem, typename Traits, typename Integer>
    std::basic_istream<Elem, Traits>& operator>>(std::basic_istream<Elem, Traits>& is, integral_io_wrapper<Integer>&& wrapper)
    {
        wrapper.input(is);
        return is;
    }


    // Main public interface:
    template <typename Integer>
    integral_output_wrapper<Integer> as_integer(const Integer& value)
    {
        return integral_output_wrapper<Integer>(value);
    }

    template <typename Integer>
    integral_io_wrapper<Integer> as_integer(Integer& value)
    {
        return integral_io_wrapper<Integer>(value);
    }
}
