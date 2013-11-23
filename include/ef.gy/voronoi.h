/**\file
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

#if !defined(EF_GY_VORONOI_H)
#define EF_GY_VORONOI_H

#include <ef.gy/geometry.h>
#include <ef.gy/set.h>

#include <set>

namespace efgy
{
    namespace geometry
    {
        template <typename S, unsigned int boundingBoxSize = 1000>
        class voronoi
        {
            public: 
                typedef typename S::vector vector;
                typedef typename S::scalar scalar;
                typedef typename S::base base;

                class cell
                {
                    public:
                        cell (void)
                            : site(), area()
                            {}

                        cell (const vector &pSite, const polygon<S> &pArea)
                            : site(pSite), area(pArea)
                            {}

                        cell (const vector &pSite, const polygon<S> &pArea, const typename colour::HSLA<base>::value &pColour)
                            : site(pSite), area(pArea), colour(pColour)
                            {
                                area.colour = pColour;
                            }

                        cell (const vector &pSite, const typename colour::HSLA<base>::value &pColour)
                            : site(pSite), colour(pColour)
                            {}

                        operator vector (void) const
                        {
                            return site;
                        }

                        operator polygon<S> (void) const
                        {
                            return area;
                        }

                        bool operator == (const cell &b) const
                        {
                            return site == b.site;
                        }

                        vector site;
                        polygon<S> area;
                        typename colour::HSLA<base>::value colour;
                };

                voronoi (void)
                    : cells()
                    {}

                voronoi (const voronoi &pV)
                    : cells(pV.cells)
                    {}

                voronoi &operator = (const voronoi &pV)
                    {
                        cells = pV.cells;
                        return *this;
                    }

                voronoi operator + (const vector &v) const
                {
                    return *this + cell (v, typename colour::HSLA<base>::value());
                }

                voronoi operator + (const cell &c) const
                {
                    const vector &v = c.site;
                    const typename colour::HSLA<base>::value &colour = c.colour;

                    voronoi r = *this;

                    if (r.cells.data.size() > 0)
                    {
                        int nearestCell = -1;
                        vector nearest;
                        maybe<scalar> nearestDistance;
                        bool haveMatch = false;

                        for (unsigned int i = 0; !haveMatch && (i < r.cells.data.size()); i++)
                        {
                            /*
                            scalar distance = vector(r.cells.data[i].site - v).lengthSquared();

                            if (!bool(nearestDistance) || (scalar(distance) < scalar(nearestDistance)))
                            {
                                nearestDistance = maybe<scalar>(distance);
                                nearestCell = i;

                                haveMatch = (polygon<S>(r.cells.data[nearestCell]) && v);
                            }*/

                            nearestCell = i;
                            haveMatch = (polygon<S>(r.cells.data[nearestCell]) && v);
                        }

                        if (!haveMatch)
                        {
                            /* new point seems to be outside the bounding box... */
                            return *this;
                        }

                        nearest = r.cells.data[nearestCell];

                        ngon<S,2> perpendicularBisector;
                        line<S> l = line<S>(v, nearest);
                        vector d = v - nearest;

                        vector m = l.midpoint();
                        vector p = euclidian::getPerpendicular<base>(d);

                        perpendicularBisector[0] = m + p;
                        perpendicularBisector[1] = m - p;

                        polygon<S> nearestPoly = r.cells.data[nearestCell];

                        std::array<maybe<polygon<S> >,3> rd = nearestPoly / perpendicularBisector;

                        if (rd[0] && rd[1])
                        {
                            const polygon<S> &rA = rd[0];
                            const polygon<S> &rB = rd[1];
                            polygon<S> rC = rd[2];

                            polygon<S> newCell;

                            if (rA && nearest)
                            {
                                r.cells.data[nearestCell].area = rA;
                                r.cells.data[nearestCell].area.colour = r.cells.data[nearestCell].colour;
                                newCell = rB;
                            }
                            else if (rB && nearest)
                            {
                                r.cells.data[nearestCell].area = rB;
                                r.cells.data[nearestCell].area.colour = r.cells.data[nearestCell].colour;
                                newCell = rA;
                            }
                            else
                            {
                                r.cells = r.cells + cell(v, rB, colour);
                                return r;
                            }

                            /* adjust adjacent cells */
                            std::set<int> usedCells = std::set<int>();

                            usedCells.insert(nearestCell);

                            while (rC.data.size() > 0)
                            {
                                vector q = rC.data[0];

                                for (unsigned int k = 0; k < r.cells.data.size(); k++)
                                {
                                    if (usedCells.count(k) == 1)
                                    {
                                        continue;
                                    }

                                    vector u = r.cells.data[k];
                                    nearestPoly = r.cells.data[k];

                                    if (nearestPoly && q)
                                    {
                                        usedCells.insert(k);

                                        l = line<S>(v, u);
                                        d = v - u;

                                        m = l.midpoint();
                                        p = euclidian::getPerpendicular<base>(d);

                                        perpendicularBisector[0] = m + p;
                                        perpendicularBisector[1] = m - p;

                                        rd = nearestPoly / perpendicularBisector;
                                        if (rd[0] && rd[1])
                                        {
                                            const polygon<S> &rD = rd[0];
                                            const polygon<S> &rE = rd[1];
                                            const polygon<S> &rF = rd[2];

                                            if (rD && vector(r.cells.data[k]))
                                            {
                                                r.cells.data[k].area = rD;
                                                r.cells.data[k].area.colour = r.cells.data[k].colour;
                                                newCell = newCell + rE;
                                            }
                                            else if (rE && vector(r.cells.data[k]))
                                            {
                                                r.cells.data[k].area = rE;
                                                r.cells.data[k].area.colour = r.cells.data[k].colour;
                                                newCell = newCell + rD;
                                            }

                                            for (unsigned int j = 0; j < rF.data.size(); j++)
                                            {
                                                rC = rC + rF.data[j];
                                            }
                                        }
                                    }
                                }

                                rC = rC - q;
                            }

                            r.cells = r.cells + cell(v, newCell, colour);
                        }
                    }
                    else
                    {
                        ngon<S,4> g = ngon<S,4>();

                        g[0][0] = v[0] - scalar(boundingBoxSize);
                        g[0][1] = v[1] - scalar(boundingBoxSize);

                        g[1][0] = v[0] + scalar(boundingBoxSize);
                        g[1][1] = v[1] - scalar(boundingBoxSize);

                        g[2][0] = v[0] + scalar(boundingBoxSize);
                        g[2][1] = v[1] + scalar(boundingBoxSize);

                        g[3][0] = v[0] - scalar(boundingBoxSize);
                        g[3][1] = v[1] + scalar(boundingBoxSize);

                        r.cells = r.cells + cell(v, g, colour); 
                    }

                    return r;
                }

                operator object<S> (void) const
                {
                    object<S> rv;

                    return rv;
                }

                math::set<cell> cells;
        };
    };
};

#endif
