/**\file
 * \brief Geometry type factory
 *
 * New geometric primitives are added ever so often and adding new types always
 * seems to require changing minor details in the frontend. The templates in
 * this class are an attempt at providing a simple interface to the primitives
 * that does not require application code, such as Topologic, to know which ids
 * belong to which primitive templates.
 *
 * Adding new types will still require recompiling an application, but it
 * should no longer require code changes other than UI cosmetics.
 *
 * \copyright
 * Copyright (c) 2012-2013, ef.gy Project Members
 * \copyright
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * \copyright
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * \copyright
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * \see Project Documentation: http://ef.gy/documentation/libefgy
 * \see Project Source Code: http://git.becquerel.org/jyujin/libefgy.git
 */

#if !defined(EF_GY_FACTORY_H)
#define EF_GY_FACTORY_H

#include <ef.gy/render-null.h>
#include <ef.gy/parametric.h>
#include <ef.gy/flame.h>

namespace efgy
{
    namespace geometry
    {
        namespace functor
        {
            template<typename Q, unsigned int d, unsigned int e, template <class,unsigned int,class,unsigned int> class T>
            class echo
            {
                public:
                    typedef std::ostream &argument;

                    bool operator () (argument &out)
                    {
                        out << d << "-" << T<Q,d,render::null<Q,e>,e>::id() << "@" << e << "\n";
                        return true;
                    }
            };
        };

        template<typename Q, unsigned int d, unsigned int e, template <class,unsigned int,class,unsigned int> class T,
                 template<typename, unsigned int, unsigned int, template <class,unsigned int,class,unsigned int> class> class func>
        class model
        {
            public:
                static bool set (const unsigned int &dims, const unsigned int &rdims, typename func<Q,d,e,T>::argument &arg)
                {
                    if (d < T<Q,d,render::null<Q,e>,e>::modelDimensionMinimum)
                    {
                        return (dims == 0);
                    }

                    if (   (T<Q,d,render::null<Q,e>,e>::modelDimensionMaximum > 0)
                        && (d > T<Q,d,render::null<Q,e>,e>::modelDimensionMaximum))
                    {
                        return model<Q,d-1,e,T,func>::set (dims, rdims, arg);
                    }

                    if (e < T<Q,d,render::null<Q,e>,e>::renderDimensionMinimum)
                    {
                        return (rdims == 0);
                    }

                    if (   (T<Q,d,render::null<Q,e>,e>::renderDimensionMaximum > 0)
                        && (e > T<Q,d,render::null<Q,e>,e>::renderDimensionMaximum))
                    {
                        return model<Q,d,e-1,T,func>::set (dims, rdims, arg);
                    }

                    if (rdims == 0)
                    {
                        if (dims == 0)
                        {
                            func<Q,d,e,T>()(arg);
                            (void) model<Q,d,e-1,T,func>::set (dims, rdims, arg);
                            return model<Q,d-1,e,T,func>::set (dims, rdims, arg);
                        }
                        else if (d == dims)
                        {
                            func<Q,d,e,T>()(arg);
                            return model<Q,d,e-1,T,func>::set (dims, rdims, arg);
                        }
                        else if (d < dims)
                        {
                            return (dims == 0);
                        }
                        else
                        {
                            return model<Q,d-1,e,T,func>::set (dims, rdims, arg);
                        }
                    }
                    else if (e == rdims)
                    {
                        if (dims == 0)
                        {
                            func<Q,d,e,T>()(arg);
                            return model<Q,d-1,e,T,func>::set (dims, rdims, arg);
                        }
                        else if (d == dims)
                        {
                            func<Q,d,e,T>()(arg);
                            return model<Q,d,e-1,T,func>::set (dims, rdims, arg);
                        }
                        else if (d < dims)
                        {
                            return (dims == 0);
                        }
                        else
                        {
                            return model<Q,d-1,e,T,func>::set (dims, rdims, arg);
                        }
                    }
                    else if (e < rdims)
                    {
                        return (rdims == 0);
                    }
                    else
                    {
                        return model<Q,d,e-1,T,func>::set (dims, rdims, arg);
                    }
                }
        };

        template<typename Q, unsigned int d, template <class,unsigned int,class,unsigned int> class T,
                 template<typename, unsigned int, unsigned int, template <class,unsigned int,class,unsigned int> class> class func>
        class model<Q,d,2,T,func>
        {
            public:
                static bool set (const unsigned int &, const unsigned int &rdims, typename func<Q,d,2,T>::argument &)
                {
                    return (rdims == 0);
                }
        };

        template<typename Q, unsigned int e, template <class,unsigned int,class,unsigned int> class T,
                 template<typename, unsigned int, unsigned int, template <class,unsigned int,class,unsigned int> class> class func>
        class model<Q,1,e,T,func>
        {
            public:
                static bool set (const unsigned int &dims, const unsigned int &, typename func<Q,1,e,T>::argument &)
                {
                    return (dims == 0);
                }
        };
    };
};

#endif
