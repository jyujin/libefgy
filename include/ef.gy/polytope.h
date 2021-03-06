/**\file
 * \brief Basic primitives
 *
 * This file contains code to create the meshes for basic primitives and to
 * present the results with the renderers provided by this library.
 *
 * 'Basic' primitives include cubes and simplices.
 *
 * \see parametric.h, ifs.h and flame.h for more complicated shapes, some of
 *      which are based on these primitives.
 *
 * \copyright
 * This file is part of the libefgy project, which is released as open source
 * under the terms of an MIT/X11-style licence, described in the COPYING file.
 *
 * \see Project Documentation: https://ef.gy/documentation/libefgy
 * \see Project Source Code: https://github.com/ef-gy/libefgy
 * \see Licence Terms: https://github.com/ef-gy/libefgy/blob/master/COPYING
 */

#if !defined(EF_GY_POLYTOPE_H)
#define EF_GY_POLYTOPE_H

#include <ef.gy/colour-space-rgb.h>
#include <ef.gy/euclidian.h>
#include <ef.gy/exponential.h>
#include <ef.gy/polar.h>
#include <ef.gy/range.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <iterator>
#include <set>
#include <type_traits>
#include <vector>

namespace efgy {
namespace geometry {
/**\defgroup libefgy-geometric-primitives Geometric Primitives
 * \brief Class templates that define basic geometric primitives
 *
 * This group contains class templates that calculate meshes for basic
 * geometric primitives, such as simplices, cubes or spheres.
 */

/**\ingroup libefgy-geometric-primitives
 * \defgroup libefgy-extended-geometric-primitives Extended Geometric Primitives
 * \brief Class templates that define extended geometric primitives
 *
 * Contains "extended" primitives - either those that reuse "basic"
 * primitives, those that are very specialised or those that can get
 * very complex.
 */

/**\brief Geometry parameters
 *
 * Contains parameters used by the shape-creating templates. You need
 * to have at least one of these for each data type you intend to use.
 *
 * \tparam Q Base data type for calculations.
 */
template <typename Q>
class parameters {
 public:
  /**\brief Construct with default parameters
   *
   * Initialises the parameter object with sane defaults for the
   * individual parameters.
   */
  parameters(void)
      : radius(1),
        radius2(0.5),
        constant(0.9),
        precision(3),
        iterations(4),
        functions(3),
        seed(0),
        preRotate(1),
        postRotate(0),
        flameCoefficients(3),
        vertexLimit(1000000),
        colourMap({{1, 0, 0}, {0, 1, 0}, {0, 0, 1}}) {}

  /**\brief Radius
   *
   * The radius of the object to be created; used by cubes,
   * simplices, spheres, most parametric surfaces, planes and just
   * about any instance of the IFS code.
   */
  Q radius;

  /**\brief Secondary radius
   *
   * Some primitives, such as tori, require two radii, so this is
   * that second radius.
   */
  Q radius2;

  /**\brief Constant
   *
   * Some formulae, e.g. the one for klein bottles, employ an
   * additional, unnamed constant that determines part of the
   * geometry.
   */
  Q constant;

  /**\brief Precision
   *
   * The precision that an object should be calculated in. Mostly
   * used to determine the smoothness of round surfaces, such as
   * those of spheres or parametric functions.
   */
  Q precision;

  /**\brief Number of iterations
   *
   * The iterations that should be used to calculate an object;
   * used primarily by the IFS code to determine the target
   * fidelity of the rendered object.
   */
  unsigned int iterations;

  /**\brief Number of functions
   *
   * Contains the number of functions for random iterated function
   * systems or fractal flames.
   */
  unsigned int functions;

  /**\brief PRNG seed
   *
   * The seed for any PRNG that is employed when creating a mesh;
   * primarily used by the random iterated function systems and
   * the random fractal flames.
   */
  unsigned int seed;

  /**\brief Allow pre-transformation rotations
   *
   * Used when generating random iterated function systems to
   * allow for a random rotation to be applied before the random
   * translation.
   */
  bool preRotate;

  /**\brief Allow post-transformation rotations
   *
   * Used when generating random iterated function systems to
   * allow for a random rotation to be applied after the random
   * translation.
   */
  bool postRotate;

  /**\brief Distinct nonzero flame coefficients
   *
   * How many distinct, nonzero flame variation coefficients to
   * allow when generating random fractal flames.
   */
  unsigned int flameCoefficients;

  /**\brief Vertex limit
   *
   * How many vertices to allow at most when generating geometry.
   * Different models deal differently with the vertex limit, but
   * generally the precision or number of iterations is reduced
   * when creating geometry that would exceed this limit.
   */
  unsigned long long vertexLimit;

  std::vector<math::vector<Q, 3, math::format::RGB>> colourMap;
};

/**\brief Flags for geometry parameters.
 *
 * Used when specifying which parameters a model uses. The actual flags are
 * template parameters, to allow for easier type aliasing and so that there does
 * not need to be any subclassing or instantiating of this class.
 */
template <bool tRadius = false, bool tRadius2 = false, bool tConstant = false,
          bool tPrecision = false, bool tIterations = false,
          bool tFunctions = false, bool tSeed = false, bool tPreRotate = false,
          bool tPostRotate = false, bool tFlameCoefficients = false>
class parameterFlags {
 public:
  static const bool radius = tRadius;
  static const bool radius2 = tRadius2;
  static const bool constant = tConstant;
  static const bool precision = tPrecision;
  static const bool iterations = tIterations;
  static const bool functions = tFunctions;
  static const bool seed = tSeed;
  static const bool preRotate = tPreRotate;
  static const bool postRotate = tPostRotate;
  static const bool flameCoefficients = tFlameCoefficients;
};

/**\brief Dimensional constraints
 *
 * This class is used to hold dimensional constraints, which are in turn
 * used by the geometric model factory to figure out which dimensions to
 * allow a model to be instantiated in.
 *
 * These constraints have to be expressed at compile time using template
 * parameters. Using '0' in modelMax and renderMax means that there are
 * no constraints on the maximum number of supported dimensions.
 *
 * \note These parameters commonly depend on the model or render depth
 *       of a model, so you will quite likely have to make these
 *       parameters dependant on the depth of the model or the target
 *       render depth. For example, if you had some kind of cube to be
 *       rendered with a render depth of 'n', then you would have to set
 *       the modelMax template argument to 'n', because you can't render
 *       an (n+1)-cube in only 'n' dimensions.
 *
 * \tparam modelMin  Minimum number of model dimensions needed.
 * \tparam modelMax  Maximum number of model dimensions supported.
 */
template <std::size_t modelMin = 2, std::size_t modelMax = 0>
class dimensions {
 public:
  /**\brief Minimum number of model dimensions needed
   *
   * Exports the 'modelMin' parameter, which is the number of
   * dimensions that the model needs to render properly.
   */
  static const std::size_t modelDimensionMinimum = modelMin;

  /**\brief Maximum number of model dimensions supported
   *
   * Exports the 'modelMax' parameter, which is the number of
   * dimensions that the model could be rendered in.
   */
  static const std::size_t modelDimensionMaximum = modelMax;
};

/**\brief Object base template
 *
 * Contains the base definitions for any polytope, i.e. code that
 * should be shared among most class templates that generate meshes to
 * be rendered.
 *
 * \tparam Q      Base type for calculations; should be a rational type
 * \tparam od     Model depth, e.g. '2' for a square or '3' for a cube
 * \tparam d      Number of dimensions of the vector space to use
 * \tparam f      Number of vertices for mesh faces
 * \tparam Format Vector coordinate format to work in, e.g.
 *                math::format::cartesian.
 */
template <typename Q, std::size_t od, std::size_t d, std::size_t f,
          typename Format>
class object {
 public:
  /**\brief Vector format type
   *
   * Holds the vector format type that was passed as an argument
   * to the template.
   */
  using format = Format;

  /**\brief Construct with renderer and parameters
   *
   * Initialises an instance of the class using a renderer
   * instance and a set of parameters. The base template does not
   * generate an actual mesh, however the class does contain
   * everything needed to render an actual mesh, so derived
   * classes should easily be able to fill in the blanks.
   *
   * \param[in]  pParameter Polytope parameters to apply.
   * \param[in]  pFormat    Coordinate format tag instance.
   */
  object(const parameters<Q> &pParameter, const format &pFormat = format())
      : parameter(pParameter), tag(pFormat) {}

  /**\brief Number of face vertices
   *
   * An alias for the 'f' template parameter, which contains the
   * number of vertices that make up an individual 'face' of the
   * resulting mesh.
   */
  static constexpr const std::size_t faceVertices = f;

  /**\brief Query the model's depth
   *
   * This static method returns a model's "depth". The exact
   * meaning of this value depends on the model in question, but
   * generally it's the number of dimensions of the surface of a
   * mesh.
   *
   * For example, the depth of a 2-sphere is "2", because while
   * you do need 3 dimensions to render a 2-sphere, the surface
   * of such a sphere is 2 dimensional since you can describe any
   * point on the surface with its (u,v) coordinates.
   *
   * A 3-cube, on the other hand, is also renderable in 3
   * dimensions, but there's no natural way to express a point on
   * the surface of such a cube with only two coordinates - you
   * could of course unroll the sides onto a texture, but it
   * still isn't "natural".
   *
   * Yeah the definition is kind of foggy; as a rule of thumb:
   * have a look at how the models are described in other places,
   * if it's called an n-something, then this 'depth' here is
   * 'n'.
   *
   * \returns The model's depth.
   */
  static constexpr const std::size_t depth = od;

  /**\brief Query the model's render depth
   *
   * This static method returns a model's render depth, which is
   * basically the number of dimensions in the vector space in
   * which it was constructed.
   *
   * \returns The model's render depth.
   */
  static constexpr const std::size_t renderDepth = d;

  /**\brief Query model ID
   *
   * Every primitive has an ID that is used to identify the type
   * at runtime. This method produces that string.
   *
   * \returns The model's ID as a C-style string.
   */
  static constexpr const char *id(void);

  /**\brief A 2D surface
   *
   * A convenient typedef for a 2D surface, commonly called a
   * 'face.'
   */
  using face = std::array<math::vector<Q, d, format>, f>;

  /**\brief Parameter reference
   *
   * A reference to the parameters used to generate the model; Set
   * in the constructor.
   */
  const parameters<Q> &parameter;

  /**\brief Coordinate format tag
   *
   * A copy of the vector format tag for this model.
   */
  const format tag;

  using usedParameters = parameterFlags<>;
};

/**\brief Polytope base template
 *
 * Separate from geometry::object to allow for easier overloads in renderers.
 *
 * \tparam Q   Base type for calculations; should be a rational type.
 * \tparam od  Model depth, e.g. '2' for a square or '3' for a cube.
 * \tparam gen Generator type.
 */
template <typename Q, std::size_t od,
          template <typename, std::size_t> class gen>
class polytope
    : public object<Q, od, gen<Q, od>::renderDepth, gen<Q, od>::faceVertices,
                    typename gen<Q, od>::format> {
 protected:
  using generator = gen<Q, od>;
  using parent = object<Q, od, generator::renderDepth, generator::faceVertices,
                        typename generator::format>;

 public:
  using parent::parent;
  using typename parent::face;
  using usedParameters = typename generator::usedParameters;
  using dimensions = typename generator::dimensions;

  using iterator = typename std::vector<face>::const_iterator;

  iterator begin(void) {
    faces = generator::faces(parent::parameter);
    return faces.begin();
  }

  iterator end(void) const { return faces.end(); }

  constexpr std::size_t size(void) const { return generator::size(); }

  static constexpr const char *id(void) { return generator::id(); }

 protected:
  /**\brief The actual mesh data
   *
   * Contains all the faces that this polytope's mesh is composed
   * of. Set via generator::faces() whenever begin() is called.
   */
  std::vector<face> faces;
};

template <class face, class iterator>
class adaptiveIterator : public iterator {
 public:
  adaptiveIterator(const iterator &pIT) : iterator(pIT) {}

  const face operator*(void)const {
    const auto &f = iterator::operator*();
    face cf;
    for (std::size_t i = 0; i < std::min(f.size(), cf.size()); i++) {
      for (std::size_t j = 0; j < std::min(f[i].size(), cf[i].size()); j++) {
        cf[i][j] = f[i][j];
      }
    }
    return cf;
  }
};

template <typename Q, std::size_t d, class model, class format>
class adapt : public object<Q, model::depth, d, model::faceVertices, format> {
 public:
  using parent = object<Q, model::depth, d, model::faceVertices, format>;
  using iterator =
      adaptiveIterator<typename parent::face, typename model::iterator>;

  adapt(const parameters<Q> &pParameter, const format &pFormat)
      : parent(pParameter, pFormat),
        object(pParameter, typename model::format()) {}

  static constexpr const char *id(void) { return model::id(); }

  iterator begin(void) { return iterator(object.begin()); }
  iterator end(void) const { return iterator(object.end()); }
  std::size_t size(void) const { return object.size(); }

  using usedParameters = typename parent::usedParameters;

 protected:
  model object;
};

template <typename Q, std::size_t d, class model, class format>
using autoAdapt = typename std::conditional<
    std::is_same<format, typename model::format>::value &&
        (d == model::renderDepth),
    model, efgy::geometry::adapt<Q, d, model, format>>::type;

namespace generators {
namespace mask {
template <std::size_t depth>
class cube {
 private:
  using vector = std::array<bool, depth>;
  using face = std::array<vector, 4>;

 public:
  static std::set<face> faces(void) {
    static std::set<face> faces{{vector({false, false}), vector({false, true}),
                                 vector({true, true}), vector({true, false})}};
    if (faces.size() == size()) {
      return faces;
    }

    for (std::size_t i = 2; i < depth; i++) {
      std::set<face> newFaces;

      for (auto fa : faces) {
        auto x = fa;

        for (std::size_t j = 0; j < 4; j++) {
          const auto &la = fa[j];
          const auto &lb = fa[((j + 1) % 4)];

          const auto &ma = la < lb ? la : lb;
          const auto &mb = la < lb ? lb : la;

          face n{{ma, mb, mb, ma}};

          n[2][i] = true;
          n[3][i] = true;

          // this should have the smallest element first, because
          //  - n[2] must be >= to n[1] because we set an extra bit
          //  - n[3] must be >= to n[0] because we set an extra bit
          //  - n[0] is <= n[1]
          newFaces.insert(n);
        }

        x[0][i] = true;
        x[1][i] = true;
        x[2][i] = true;
        x[3][i] = true;

        // since we moved this face in the new direction, we reverse the winding
        // direction in order to reverse the normal.
        std::reverse(x.begin(), x.end());

        // this makes sure that the smallest vertex (via min_element) is first.
        // std::rotate(x.begin(), std::min_element(x.begin(), x.end()),
        // x.end());

        newFaces.insert(x);
      }

      faces.insert(newFaces.begin(), newFaces.end());
    }

    return faces;
  }

  /**\brief Number of surfaces
   *
   * This is the number of 2D surfaces that the hypercube has. It helps to know
   * this when trying to impose a limit on the number of vertices in certain
   * derived classes.
   *
   * \note The general, closed formula for this is (n being the depth of the
   *     cube): (2^(n-3))*(n-1)*n
   */
  static constexpr std::size_t size(void) {
    return std::pow<long>(2, (long)depth - 3) * (long)(depth - 1) * (long)depth;
  }
};

template <>
class cube<2> {
 private:
  using vector = std::array<bool, 2>;
  using face = std::array<vector, 4>;

 public:
  static constexpr std::array<face, 1> faces(void) {
    return {vector({false, false}), vector({false, true}), vector({true, true}),
            vector({true, false})};
  }

  static constexpr std::size_t size(void) { return 1; }
};

template <>
class cube<1> {
 private:
  using vector = std::array<bool, 1>;
  using face = std::array<vector, 4>;

 public:
  static constexpr std::array<face, 0> faces(void) { return {}; }

  static constexpr std::size_t size(void) { return 0; }
};
}  // namespace mask

template <typename Q, std::size_t depth>
class cube {
 public:
  typedef dimensions<2, 0> dimensions;
  static constexpr const std::size_t renderDepth = depth;
  static constexpr const std::size_t faceVertices = 4;
  static constexpr const char *id(void) { return "cube"; }
  using usedParameters = parameterFlags<true>;
  using format = math::format::cartesian;

 private:
  using vector = math::vector<Q, renderDepth, format>;
  using face = std::array<vector, faceVertices>;
  using source = mask::cube<depth>;

 public:
  static std::vector<face> faces(const parameters<Q> &parameter) {
    std::vector<face> res(source::size(), face());
    const auto pd = parameter.radius * Q(.5);
    const auto nd = parameter.radius * Q(-.5);
    auto r = res.begin();

    for (auto fa : source::faces()) {
      for (std::size_t j = 0; j < depth; j++) {
        (*r)[0][j] = fa[0][j] ? pd : nd;
        (*r)[1][j] = fa[1][j] ? pd : nd;
        (*r)[2][j] = fa[2][j] ? pd : nd;
        (*r)[3][j] = fa[3][j] ? pd : nd;
      }
      r++;
    }

    return res;
  }

  /**\brief Number of surfaces
   *
   * This is the number of 2D surfaces that the hypercube has. It helps to know
   * this when trying to impose a limit on the number of vertices in certain
   * derived classes.
   *
   * \note The general, closed formula for this is (n being the depth of the
   *     cube): (2^(n-3))*(n-1)*n
   */
  static constexpr std::size_t size(void) { return source::size(); }
};
}  // namespace generators

/**\ingroup libefgy-geometric-primitives
 * \brief The hypercube
 *
 * This template calculates the mesh for a cube in any number of
 * dimensions. For example if you set the 'od' parameter to '3', then
 * you get a regular cube, but if you set the 'od' parameter to '2' the
 * resulting mesh is a 2D square. '4' will get you a so-called
 * "tesseract", which looks like this:
 *
 * \image html geometric-primitive-4-cube.svg "A tesseract, i.e. a 4-cube"
 *
 * Cubes are a fairly simple and common primitive, and they're easy to
 * generalise to higher dimensional spaces. They also make excellent
 * base primitives for iterated function systems, because they don't
 * produce all that much output while still covering a lot of space,
 * and deformations don't lose much information.
 *
 * In terms of parameters, this primitive only makes use of the
 * parameter::radius field, which i used to determine the size of
 * the mesh. Confusingly, this radius is actually used as the edge
 * length for historical reasons.
 *
 * \tparam Q      Base datatype for calculations
 * \tparam od     The 'depth' of the hypercube; e.g. '3' for a cube
 *
 * \see http://ef.gy/documentation/topologic - Topologic, the programme
 *      that was used to render the sample image.
 *
 * \see http://en.wikipedia.org/wiki/Hypercube for more information on
 *      hypercubes in general.
 *
 * \todo Use the parameter::radius field properly; this should
 *       probably be half the diagonal of the resulting model.
 */
template <typename Q, std::size_t od>
using cube = polytope<Q, od, generators::cube>;
}  // namespace geometry
}  // namespace efgy

#endif
