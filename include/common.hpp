/**
 * \file common.hpp
 * \brief Type definitions and utility functions
 */

#ifndef COMMON_HPP_
#define COMMON_HPP_

#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iomanip>
#include <ostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace oqs {
namespace C {
// Everything in liboqs has C linkage
extern "C" {
#include <oqs/common.h>
}
} // namespace C
using byte = std::uint8_t;        ///< byte (unsigned)
using bytes = std::vector<byte>;  ///< vector of bytes (unsigned)
using OQS_STATUS = C::OQS_STATUS; ///< bring OQS_STATUS into the oqs namespace

/**
 * \brief liboqs version string
 * \return liboqs version string
 */
inline std::string oqs_version() { return oqs::C::OQS_version(); }

/**
 * \brief liboqs-cpp version string
 * \return liboqs-cpp version string
 */
inline std::string oqs_cpp_version() { return LIBOQS_CPP_VERSION; }

/**
 * \brief Sets to zero the content of \a v by invoking the liboqs
 * OQS_MEM_cleanse() function. Use it to clean "hot" memory areas, such as
 * secret keys etc.
 * \param v Vector of bytes
 */
inline void mem_cleanse(bytes& v) { C::OQS_MEM_cleanse(v.data(), v.size()); }

/**
 * \namespace internal
 * \brief Internal implementation details
 */
namespace internal {
/* code from
https://github.com/vsoftco/qpp/blob/master/include/internal/classes/singleton.h
 */
/**
 * \class oqs::internal::Singleton
 * \brief Singleton class using CRTP pattern
 * \note Code from
 * https://github.com/softwareqinc/qpp/blob/main/include/internal/classes/singleton.hpp
 * \tparam T Class type of which instance will become a Singleton
 */
template <typename T>
class Singleton {
  protected:
    Singleton() noexcept = default;

    Singleton(const Singleton&) = delete;

    Singleton& operator=(const Singleton&) = delete;

    virtual ~Singleton() = default;

  public:
    /**
     * \brief Singleton instance (thread-safe) via CRTP pattern
     * \return Singleton instance
     */
    static T& get_instance() noexcept(std::is_nothrow_constructible<T>::value) {
        // Guaranteed to be destroyed.
        // Instantiated on first use.
        // Thread safe in C++11
        static T instance;

        return instance;
    }
}; // class Singleton

/**
 * \class oqs::internal::HexChop
 * \brief std::ostream manipulator for long vectors of oqs::byte, use it to
 * display only a small number of elements from the beginning and end of the
 * vector
 */
class HexChop {
    bytes v_; ///< vector of byes
    std::size_t from_start_,
        from_end_; ///< number of hex bytes taken from the start and from the
                   ///< end
    /**
     * \brief std::ostream manipulator
     * \param os Output stream
     * \param start Number of hex characters displayed from the beginning of
     * the vector
     * \param end  Number of hex characters displayed from the end of the vector
     * \param is_short Vector is too short, display all hex characters
     */
    void manipulate_ostream_(std::ostream& os, std::size_t start,
                             std::size_t end, bool is_short) const {
        std::stringstream ss;
        ss << std::setfill('0') << std::hex << std::uppercase;

        bool first = true;
        for (std::size_t i = 0; i < start; ++i) {
            if (first) {
                first = false;
                ss << std::setw(2) << static_cast<int>(v_[i]);
            } else {
                ss << " " << std::setw(2) << static_cast<int>(v_[i]);
            }
        }

        if (!is_short)
            ss << " ... ";

        first = true;
        std::size_t v_size = v_.size();
        for (std::size_t i = v_size - end; i < v_size; ++i) {
            if (first) {
                first = false;
                ss << static_cast<int>(v_[i]);
            } else
                ss << " " << std::setw(2) << static_cast<int>(v_[i]);
        }

        os << ss.str();
    }

  public:
    /**
     * \brief Constructs an instance of oqs::internal::HexChop
     * \param v Vector of bytes
     * \param from_start Number of hex characters displayed from the beginning
     * of the vector
     * \param from_end  Number of hex characters displayed from
     * the from_end of the vector
     */
    explicit HexChop(oqs::bytes v, std::size_t from_start, std::size_t from_end)
        : v_{std::move(v)}, from_start_{from_start}, from_end_{from_end} {}

    /**
     * \brief std::ostream extraction operator for oqs::internal::HexChop
     * \param os Output stream
     * \param rhs oqs::internal::HexChop instance
     * \return Reference to the output stream
     */
    friend std::ostream& operator<<(std::ostream& os, const HexChop& rhs) {

        bool is_short = rhs.from_start_ + rhs.from_end_ >= rhs.v_.size();
        if (is_short)
            rhs.manipulate_ostream_(os, rhs.v_.size(), 0, true);
        else
            rhs.manipulate_ostream_(os, rhs.from_start_, rhs.from_end_, false);

        return os;
    }
}; // class HexChop
} // namespace internal

/* code from
https://github.com/vsoftco/qpp/blob/master/include/classes/timer.h
 */
/**
 * \class oqs::Timer
 * \brief High resolution timer
 * \note Code from
 * https://github.com/vsoftco/qpp/blob/master/include/classes/timer.h
 * \tparam T Tics duration, default is std::chrono::duration<double>,
 * i.e. seconds in double precision
 * \tparam CLOCK_T Clock's type, default is std::chrono::steady_clock,
 * not affected by wall clock changes during runtime
 */
template <typename T = std::chrono::duration<double>,
          typename CLOCK_T = std::chrono::steady_clock>
class Timer {
  protected:
    typename CLOCK_T::time_point start_, end_;

  public:
    /**
     * \brief Constructs an instance with the current time as the start point
     */
    Timer() noexcept : start_{CLOCK_T::now()}, end_{start_} {}

    /**
     * \brief Resets the chronometer
     *
     * Resets the start/end point to the current time
     *
     * \return Reference to the current instance
     */
    Timer& tic() noexcept {
        start_ = end_ = CLOCK_T::now();

        return *this;
    }

    /**
     * \brief Stops the chronometer
     *
     * Set the current time as the end point
     *
     * \return Reference to the current instance
     */
    Timer& toc() & noexcept {
        end_ = CLOCK_T::now();

        return *this;
    }

    /**
     * \brief Time passed in the duration specified by T
     *
     * \return Number of tics (specified by T) that passed between the
     * instantiation/reset and invocation of oqs::Timer::toc()
     */
    double tics() const noexcept {
        return static_cast<double>(
            std::chrono::duration_cast<T>(end_ - start_).count());
    }

    /**
     * \brief Duration specified by U
     *
     * \tparam U Duration, default is T, which defaults to
     * std::chrono::duration<double>, i.e. seconds in double precision
     *
     * \return Duration that passed between the
     * instantiation/reset and invocation of oqs::Timer::toc()
     */
    template <typename U = T>
    U get_duration() const noexcept {
        return std::chrono::duration_cast<U>(end_ - start_);
    }

    /**
     * \brief Default virtual destructor
     */
    virtual ~Timer() = default;

    friend std::ostream& operator<<(std::ostream& os, const Timer& rhs) {
        return os << rhs.tics();
    }
}; // class Timer

/**
 * \brief Constructs an instance of oqs::internal::HexChop
 * \param v Vector of bytes
 * \param from_start Number of hex characters displayed from the beginning of
 * the vector
 * \param from_end  Number of hex characters displayed from the from_end of the
 * vector
 * \return Instance of oqs::internal::HexChop
 */
inline internal::HexChop hex_chop(const bytes& v, std::size_t from_start = 8,
                                  std::size_t from_end = 8) {
    return internal::HexChop{v, from_start, from_end};
}
} // namespace oqs

/**
 * \brief std::ostream extraction operator for oqs::bytes
 * \param os Output stream
 * \param rhs Vector of oqs::byte
 * \return Reference to the output stream
 */
inline std::ostream& operator<<(std::ostream& os, const oqs::bytes& rhs) {
    return os << oqs::hex_chop(rhs, rhs.size(), 0);
}

/**
 * \brief std::ostream extraction operator for vectors of strings
 * \param os Output stream
 * \param rhs Vector of std::string
 * \return Reference to the output stream
 */
inline std::ostream& operator<<(std::ostream& os,
                                const std::vector<std::string>& rhs) {
    std::string sep;
    for (auto&& elem : rhs) {
        os << sep << elem;
        sep = " ";
    }

    return os;
}

inline namespace oqs_literals {
/**
 * \brief User-defined literal operator for converting C-style strings to
 * oqs::bytes
 * \note The null terminator is not included
 * \param c_str C-style string
 * \param length C-style string length (deduced automatically by the compiler)
 * \return The byte representation of the input C-style string
 */
inline oqs::bytes operator""_bytes(const char* c_str, std::size_t length) {
    oqs::bytes result(length);
    for (std::size_t i = 0; i < length; ++i)
        result[i] = static_cast<oqs::byte>(c_str[i]);

    return result;
}
} // namespace oqs_literals

#endif // COMMON_HPP_
