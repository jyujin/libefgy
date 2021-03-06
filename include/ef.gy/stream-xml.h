/**\file
 * \brief XML stream tag
 *
 * Contains the XML stream tag, used to differentiate between plain output
 * streams and XML ones.
 *
 * \copyright
 * This file is part of the libefgy project, which is released as open source
 * under the terms of an MIT/X11-style licence, described in the COPYING file.
 *
 * \see Project Documentation: https://ef.gy/documentation/libefgy
 * \see Project Source Code: https://github.com/ef-gy/libefgy
 * \see Licence Terms: https://github.com/ef-gy/libefgy/blob/master/COPYING
 */

#if !defined(EF_GY_STREAM_XML_H)
#define EF_GY_STREAM_XML_H

#include <ostream>
#include <string>

namespace efgy {
namespace xml {
/**\brief std::ostream XML tag
 *
 * Used to distinguish between a plain std::ostream, and one where the
 * output should be in XML format.
 *
 * \tparam C Character type for the basic_ostream reference.
 */
template <typename C>
class ostream {
 public:
  /**\brief Construct with stream reference
   *
   * Initialises a new ostream XML tag instance.
   *
   * \param[out] pStream The stream to write to.
   */
  ostream(std::basic_ostream<C> &pStream)
      : stream(pStream), precision(24), resolution(8) {}

  /**\brief Output stream reference
   *
   * This is the stream where the output is written to.
   */
  std::basic_ostream<C> &stream;

  /**\brief Output precision
   *
   * This is the number of bits in the numerator or denominator of
   * fractions that are output to the stream. The default is '24'.
   */
  long precision;

  /**\brief Output range resolution
   *
   * This is the number of steps used when writing the elements of
   * ranges to the stream. The default is '8'.
   */
  long resolution;
};

/**\brief XML tag
 *
 * Write this to an ostream to turn it into an ostream. Like this:
 *
 * \code{.cpp}
 * cout << xml::tag();
 * \encode
 */
class tag {};

/**\brief Convert std::ostream to XML
 *
 * Converts the given stream to an XML stream so that write operations
 * after that will produce XML instead of plain text.
 *
 * \tparam C Character type for the basic_ostream reference.
 *
 * \param[out] stream The stream to write to.
 */
template <typename C>
constexpr inline ostream<C> operator<<(std::basic_ostream<C> &stream,
                                       const tag &) {
  return ostream<C>(stream);
}

/**\brief Precision wrapper
 *
 * Used to update the precision field of an XML output stream, like so:
 *
 * \code{.cpp}
 * cout << XML() << precision(24);
 * \encode
 */
class precision {
 public:
  /**\brief Construct with precision
   *
   * Initialises the instance with a precision value.
   *
   * \param[in] pPrecision The precision to store.
   */
  constexpr precision(const long &pPrecision) : value(pPrecision) {}

  /**\brief Precision value
   *
   * This is the actual precision stored in the wrapper.
   */
  const long value;
};

/**\brief Update precision of XML stream
 *
 * Updates the precision member of an XML output stream.
 *
 * \tparam C Character type for the basic_ostream reference.
 *
 * \param[in] stream The stream to modify.
 * \param[in] p      The new precision value.
 *
 * \returns A new copy of the stream.
 */
template <typename C>
constexpr inline ostream<C> operator<<(ostream<C> stream, const precision &p) {
  return stream.precision = p.value, stream;
}

/**\brief Resolution wrapper
 *
 * Used to update the resolution field of an XML output stream, like so:
 *
 * \code{.cpp}
 * cout << XML() << resolution(8);
 * \encode
 */
class resolution {
 public:
  /**\brief Construct with resolution
   *
   * Initialises the instance with a resolution value.
   *
   * \param[in] pResolution The resolution to store.
   */
  constexpr resolution(const long &pResolution) : value(pResolution) {}

  /**\brief Resolution value
   *
   * This is the actual resolution stored in the wrapper.
   */
  const long value;
};

/**\brief Update resolution of XML stream
 *
 * Updates the resolution member of an XML output stream.
 *
 * \tparam C Character type for the basic_ostream reference.
 *
 * \param[in] stream The stream to modify.
 * \param[in] p      The new resolution value.
 *
 * \returns A new copy of the stream.
 */
template <typename C>
constexpr inline ostream<C> operator<<(ostream<C> stream, const resolution &p) {
  return stream.resolution = p.value, stream;
}
}  // namespace xml
}  // namespace efgy

#endif
