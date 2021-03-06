/**\file
 * \brief Produce SVG output
 *
 * Contains classes that turn polygons in arbitrary-dimensional euclidian
 * spaces into 2D SVG vector graphics.
 *
 * \copyright
 * This file is part of the libefgy project, which is released as open source
 * under the terms of an MIT/X11-style licence, described in the COPYING file.
 *
 * \see Project Documentation: https://ef.gy/documentation/libefgy
 * \see Project Source Code: https://github.com/ef-gy/libefgy
 * \see Licence Terms: https://github.com/ef-gy/libefgy/blob/master/COPYING
 */

#if !defined(EF_GY_RENDER_SVG_H)
#define EF_GY_RENDER_SVG_H

#include <ef.gy/euclidian.h>
#include <ef.gy/polytope.h>
#include <ef.gy/projection.h>
#include <ef.gy/stream-svg.h>

#include <sstream>

namespace efgy {
namespace render {
/**\brief Render to SVG
 *
 * Converts objects to strings so that an SVG parser can understand and
 * display them properly.
 *
 * \tparam Q The base numeric type you intend to use.
 * \tparam d The number of dimensions for vectors.
 */
template <typename Q, unsigned int d>
class svg {
 public:
  /**\brief Construct with matrices
   *
   * Constructs an svg converter with references to a
   * transformation matrix, a projection matrix and an additional
   * lower-dimensional renderer that values to project are passed
   * to.
   *
   * \param[in]  pTransformation An affine transformation matrix
   *    to pre-multiply vectors with when converting them to SVG.
   * \param[in]  pProjection A projective transformation that is
   *    used to reduce the number of dimensions so the vector can
   *    be passed to pLowerRenderer.
   * \param[out] pLowerRenderer An instance of this template with
   *    one spatial dimension less - used in a chain to get from
   *    an arbitrary number of dimensions to 2 dimensions, as
   *    SVGs only support 2D graphics.
   */
  svg(const geometry::transformation::affine<Q, d> &pTransformation,
      const geometry::projection<Q, d> &pProjection,
      svg<Q, d - 1> &pLowerRenderer)
      : transformation(pTransformation),
        projection(pProjection),
        lowerRenderer(pLowerRenderer) {}

  /**\brief Begin drawing a new frame
   *
   * Reset the renderer's state and start drawing a new image.
   * Call this before using any drawing commands for the first
   * time, or when drawing an entirely new image.
   */
  void frameStart(void) {
    combined = transformation * projection;
    lowerRenderer.frameStart();
  };

  /**\brief End drawing current frame
   *
   * Finalise the renderer's state. Not used by the SVG renderer.
   */
  void frameEnd(void) const {};

  /**\brief Draw polygon
   *
   * Draw a polygon with q vertices. The Polygon should be
   * convex; if it isn't then you'll get rather strange results.
   *
   * \tparam q The number of vertices that define the polygon.
   * \tparam C Character type for the basic_ostream reference.
   *
   * \param[out] output Where to write the polygon to.
   * \param[in]  pV     The vertices that define the polygon.
   */
  template <std::size_t q, typename C>
  void draw(std::basic_ostream<C> &output,
            const std::array<math::vector<Q, d>, q> &pV) const {
    std::array<math::vector<Q, d - 1>, q> V;

    for (unsigned int i = 0; i < q; i++) {
      V[i] = combined * pV[i];
    }

    lowerRenderer.draw(output, V);
  }

 protected:
  /**\brief Affine transformation matrix
   *
   * This is a reference to the affine transformation matrix that
   * any vectors are pre-multiplied with.
   */
  const geometry::transformation::affine<Q, d> &transformation;

  /**\brief Projective transformation
   *
   * This is a reference to the projective transformation that is
   * used to decrease any vector's number of dimensions by one,
   * so that the lower renderer can do its job.
   */
  const geometry::projection<Q, d> &projection;

  /**\brief Combined projective transformation
   *
   * Calculated by frameStart() to speed up rendering by merging
   * the affine pre-transformation and the projective matrices.
   */
  geometry::transformation::projective<Q, d> combined;

  /**\brief Lower renderer
   *
   * A reference to the renderer that drawing commands are passed
   * to; SVG renderers are chained together so that you can
   * render objects with arbitrarily large numbers of spatial
   * dimensions, even though SVGs only support 2D output.
   */
  svg<Q, d - 1> &lowerRenderer;
};

/**\brief Render to SVG (2D fix point)
 *
 * Converts objects to strings so that an SVG parser can understand and
 * display them properly. This is the 2D fix point of the
 * transformation chain, which is defined separately as SVGs are 2D
 * graphics and so no additional transformations are necessary and this
 * class can produce actual output.
 *
 * \tparam Q The base numeric type you intend to use.
 */
template <typename Q>
class svg<Q, 2> {
 public:
  /**\brief Construct with transformation matrix
   *
   * Unlike the higher-dimensional version of this class, the 2D
   * version only takes one 2D affine transformation, as there is
   * no need to project the 2D vectors it handles because SVGs
   * are themselves 2D vector graphics.
   *
   * \param[in] pTransformation An affine transformation applied
   *    to any vectors rendered with this class before writing
   *    them to the output stream.
   */
  svg(const typename geometry::transformation::affine<Q, 2> &pTransformation,
      const typename geometry::transformation::projective<Q, 2> &,
      const svg<Q, 1> &)
      : transformation(pTransformation) {}

  /**\brief Begin drawing a new frame
   *
   * Reset the renderer's state and start drawing a new image.
   * Not used by the 2D SVG renderer.
   */
  void frameStart(void) const {};

  /**\brief End drawing current frame
   *
   * Finalise the renderer's state. Not used by the SVG renderer.
   */
  void frameEnd(void) const {};

  /**\brief Draw polygon
   *
   * Draw a polygon with q vertices. The Polygon should be
   * convex; if it isn't then you'll get rather strange results.
   *
   * The resulting SVG fragment is written to the 'output'
   * stream.
   *
   * \tparam q The number of vertices that define the polygon.
   * \tparam C Character type for the basic_ostream reference.
   *
   * \param[out] output Where to write the polygon to.
   * \param[in]  pV     The vertices that define the polygon.
   */
  template <std::size_t q, typename C>
  void draw(std::basic_ostream<C> &output,
            const std::array<math::vector<Q, 2>, q> &pV) const {
    output << "<path d='";
    for (unsigned int i = 0; i < q; i++) {
      std::stringstream sbuf1;
      std::stringstream sbuf2;

      const math::vector<Q, 2> V = transformation * pV[i];

      const double a0 = Q(V[0]);
      const double a1 = -Q(V[1]);

      if (i == 0) {
        sbuf1 << "M" << a0 << "," << a1;
        sbuf2 << "M" << a0 << "," << a1;
      } else {
        const math::vector<Q, 2> V1 = transformation * pV[(i - 1)];

        const double a0r = a0 - Q(V1[0]);
        const double a1r = a1 + Q(V1[1]);

        if (pV[i][1] == V1[1]) {
          sbuf1 << "H" << a0;
          sbuf2 << "h" << a0r;
        } else if (pV[i][0] == -V1[0]) {
          sbuf1 << "V" << a1;
          sbuf2 << "v" << a1r;
        } else {
          sbuf1 << "L" << a0 << "," << a1;
          sbuf2 << "l" << a0r << "," << a1r;
        }
      }
      if (sbuf1.str().size() >= sbuf2.str().size()) {
        output << sbuf2.str();
      } else {
        output << sbuf1.str();
      }
    }
    output << "Z'/>";
  }

 protected:
  /**\brief Affine transformation matrix
   *
   * This is a reference to the affine transformation matrix that
   * any vectors are pre-multiplied with.
   */
  const geometry::transformation::affine<Q, 2> &transformation;
};

template <typename Q>
class svg<Q, 1> {};

/**\brief std::ostream SVG tag
 *
 * Used to distinguish between a plain std::ostream, and one where the
 * output should be in SVG format.
 *
 * \tparam C Character type for the basic_ostream reference.
 * \tparam Q The base numeric type you intend to use.
 * \tparam d The number of dimensions for vectors.
 */
template <typename C, typename Q, unsigned int d>
class osvgstream {
 public:
  /**\brief Construct with stream reference
   *
   * Initialises a new ostream SVG tag instance.
   *
   * \param[out] pStream The stream to write to.
   * \param[in]  pRender The renderer instance to use.
   */
  osvgstream(std::basic_ostream<C> &pStream, const svg<Q, d> &pRender)
      : stream(pStream), render(pRender) {}

  /**\brief Output stream reference
   *
   * This is the stream where the output is written to.
   */
  std::basic_ostream<C> &stream;

  /**\brief SVG renderer reference
   *
   * This is the SVG renderer instance that things will be
   * rendered with, if the input requires that.
   */
  const svg<Q, d> &render;
};

/**\brief Convert std::ostream to SVG
 *
 * Converts the given stream to an SVG stream so that write operations
 * after that will produce SVG instead of plain text.
 *
 * \tparam C Character type for the basic_ostream reference.
 * \tparam Q The base numeric type you intend to use.
 * \tparam d The number of dimensions for vectors.
 *
 * \param[out] stream The stream to write to.
 * \param[in]  render The SVG renderer to project with.
 *
 * \returns A new osvgstream with the given parameters.
 */
template <typename C, typename Q, unsigned int d>
constexpr inline osvgstream<C, Q, d> operator<<(std::basic_ostream<C> &stream,
                                                const svg<Q, d> &render) {
  return osvgstream<C, Q, d>(stream, render);
}

/**\brief Write out polytope as SVG
 *
 * Iterates through all of the polytope's faces and then writes a 2D
 * projection of them to the stream.
 *
 * \tparam C      Character type for the basic_ostream reference.
 * \tparam Q      Base type for calculations; should be a rational type
 * \tparam od     Model depth, e.g. '2' for a square or '3' for a cube
 * \tparam d      Number of dimensions of the vector space to use
 * \tparam f      Number of vertices for mesh faces
 * \tparam format Vector coordinate format to work in, e.g.
 *                math::format::cartesian.
 *
 * \param[out] stream The stream to write to.
 * \param[in]  poly   The polytope to write out as an SVG.
 *
 * \returns A new copy of the stream that was passed in.
 */
template <typename C, typename Q, unsigned int d, typename model>
static inline osvgstream<C, Q, d> operator<<(osvgstream<C, Q, d> stream,
                                             model &poly) {
  for (const auto &p : poly) {
    std::array<math::vector<Q, d>, model::faceVertices> q;

    for (std::size_t i = 0; i < poly.faceVertices; i++) {
      q[i] = p[i];
    }

    stream.render.draw(stream.stream, q);
  }

  return stream;
}
}  // namespace render
}  // namespace efgy

#endif
