/*
 * This file is part of the ef.gy project.
 * See the appropriate repository at http://ef.gy/.git for exact file
 * modification records.
*/

/*
 * Copyright (c) 2012-2013, ef.gy Project Members
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
*/

#if !defined(EF_GY_TRANSFORMATION_H)
#define EF_GY_TRANSFORMATION_H

#include <ef.gy/euclidian.h>
#include <ef.gy/matrix.h>

namespace efgy
{
    namespace geometry
    {
        namespace transformation
        {
            template <typename Q, unsigned int d> class projective;

            template <typename Q, unsigned int d>
            class linear
            {
                public:
                    linear ()
                        {
                            for (unsigned int i = 0; i < d; i++)
                            {
                                for (unsigned int j = 0; j < d; j++)
                                {
                                    matrix.data[i][j] = (i == j) ? 1 : 0;
                                }
                            }
                        }

                    typename euclidian::space<Q,d>::vector operator *
                        (const typename euclidian::space<Q,d>::vector &pV) const
                    {
                        typename euclidian::space<Q,d>::vector rv;

                        math::matrix<Q,1,d> vectorMatrix;

                        for (unsigned int i = 0; i < d; i++)
                        {
                            vectorMatrix.data[0][i] = pV.data[i];
                        }

                        vectorMatrix
                            = vectorMatrix
                            * matrix;

                        for (unsigned int i = 0; i < d; i++)
                        {
                            rv.data[i] = vectorMatrix.data[0][i];
                        }

                        return rv;
                    }

                    linear operator *
                        (const linear &pB) const
                    {
                        linear t;
                        t.matrix = this->matrix * pB.matrix;
                        return t;
                    }

                    projective<Q,d> operator *
                        (const projective<Q,d> &pB) const
                    {
                        return pB * (*this);
                    }

                    math::matrix<Q,d,d> matrix;
            };

            template <typename Q, unsigned int d>
            class identity : public linear<Q,d>
            {
                public:
                    identity()
                        : linear<Q,d>()
                        {}
            };

            template <typename Q, unsigned int d>
            class affine
            {
                public:
                    affine ()
                        {
                            for (unsigned int i = 0; i <= d; i++)
                            {
                                for (unsigned int j = 0; j <= d; j++)
                                {
                                    transformationMatrix.data[i][j] = (i == j) ? 1 : 0;
                                }
                            }
                        }

                    affine (const linear<Q,d> &L)
                        {
                            for (unsigned int i = 0; i < d; i++)
                            {
                                for (unsigned int j = 0; j < d; j++)
                                {
                                    transformationMatrix.data[i][j] = L.data[i][j];
                                }
                            }
                            for (unsigned int i = 0; i < d; i++)
                            {
                                transformationMatrix.data[i][d] = Q(0);
                                transformationMatrix.data[d][i] = Q(0);
                            }
                            transformationMatrix.data[d][d] = Q(1);
                        }

                    typename euclidian::space<Q,d>::vector operator *
                        (const typename euclidian::space<Q,d>::vector &pV) const
                    {
                        typename euclidian::space<Q,d>::vector rv;

                        math::matrix<Q,1,d+1> vectorMatrix;

                        for (unsigned int i = 0; i < d; i++)
                        {
                            vectorMatrix.data[0][i] = pV.data[i];
                        }

                        vectorMatrix.data[0][d] = 1;

                        vectorMatrix
                            = vectorMatrix
                            * transformationMatrix;

                        for (unsigned int i = 0; i < d; i++)
                        {
                            rv.data[i] = vectorMatrix.data[0][i] / vectorMatrix.data[0][d];
                        }

                        return rv;
                    }

                    affine operator *
                        (const affine &pB) const
                    {
                        affine t;
                        t.transformationMatrix = this->transformationMatrix * pB.transformationMatrix;
                        return t;
                    }

                    projective<Q,d> operator *
                        (const projective<Q,d> &pB) const
                    {
                        projective<Q,d> t;
                        t.transformationMatrix = this->transformationMatrix * pB.transformationMatrix;
                        return t;
                    }

                    math::matrix<Q,d+1,d+1> transformationMatrix;
            };

            template <typename Q, unsigned int d>
            class projective : public affine<Q,d>
            {
                public:
                    projective ()
                        : affine<Q,d>()
                        {}

                    typename euclidian::space<Q,(d-1)>::vector operator *
                        (const typename euclidian::space<Q,d>::vector &pP) const
                    {
                        typename euclidian::space<Q,(d-1)>::vector result;
                    
                        typename euclidian::space<Q,d>::vector R = affine<Q,d>(*this) * pP;
                    
                        Q S = 1 / R.data[(d-1)];
                    
                        for (unsigned int i = 0; i < (d-1); i++)
                        {
                            result.data[i] = S * R.data[i];
                        }
                    
                        return result;
                    }

                    projective operator *
                        (const affine<Q,d> &pB) const
                    {
                        projective<Q,d> t;
                        t.transformationMatrix = this->transformationMatrix * pB.transformationMatrix;
                        return t;
                    }

                    using affine<Q,d>::transformationMatrix;
            };

            template <typename Q, unsigned int d>
            class scale : public affine<Q,d>
            {
                public:
                    scale(const Q &pScale)
                        : targetScale(pScale)
                        {
                            updateMatrix();
                        }

                    void updateMatrix (void)
                    {
                        for (unsigned int i = 0; i <= d; i++)
                        {
                            for (unsigned int j = 0; j <= d; j++)
                            {
                                if (i == j)
                                {
                                    transformationMatrix.data[i][j] = i == d ? Q(1) / targetScale : Q(1);
                                }
                                else
                                {
                                    transformationMatrix.data[i][j] = 0;
                                }
                            }
                        }
                    }

                    using affine<Q,d>::transformationMatrix;

                protected:
                    const Q &targetScale;
            };

            template <typename Q, unsigned int d>
            class rotation : public affine<Q,d>
            {
                public:
                    rotation(const Q &pAngle, const unsigned int &pAxis1, const unsigned int &pAxis2)
                        : angle(pAngle), axis1(pAxis1), axis2(pAxis2)
                        {
                            updateMatrix();
                        }
                
                    void updateMatrix (void)
                    {
                        for (unsigned int i = 0; i <= d; i++)
                        {
                            for (unsigned int j = 0; j <= d; j++)
                            {
                                if ((i == axis1) && (j == axis1))
                                {
                                    transformationMatrix.data[i][j] =  cos(angle);
                                }
                                else if ((i == axis1) && (j == axis2))
                                {
                                    transformationMatrix.data[i][j] = -sin(angle);
                                }
                                else if ((i == axis2) && (j == axis2))
                                {
                                    transformationMatrix.data[i][j] =  cos(angle);
                                }
                                else if ((i == axis2) && (j == axis1))
                                {
                                    transformationMatrix.data[i][j] =  sin(angle);
                                }
                                else if (i == j)
                                {
                                    transformationMatrix.data[i][j] = Q(1);
                                }
                                else
                                {
                                    transformationMatrix.data[i][j] = 0;
                                }
                            }
                        }
                        
                        if ((axis1 + axis2 + d + 1) % 2 == 1)
                        {
                            transformationMatrix = math::transpose(transformationMatrix);
                        }
                    }

                    using affine<Q,d>::transformationMatrix;

                protected:
                    const Q &angle;
                    const unsigned int &axis1;
                    const unsigned int &axis2;
            };

            template <typename Q, unsigned int d>
            class translation : public affine<Q,d>
            {
                public:
                    translation(const typename euclidian::space<Q,d>::vector &pFrom)
                        : from(pFrom)
                        {
                            updateMatrix();
                        }

                    void updateMatrix (void)
                    {
                        if (d == 3)
                        {
                            transformationMatrix.data[3][0] = from.data[0];
                            transformationMatrix.data[3][1] = from.data[1];
                            transformationMatrix.data[3][2] = from.data[2];
                        }
                        else for (unsigned int i = 0; i <= d; i++)
                        {
                            for (unsigned int j = 0; j <= d; j++)
                            {
                                if ((i == d) && (j < d))
                                {
                                    transformationMatrix.data[i][j] = from.data[j];
                                }
                                else if (i == j)
                                {
                                    transformationMatrix.data[i][j] = 1;
                                }
                                else
                                {
                                    transformationMatrix.data[i][j] = 0;
                                }
                            }
                        }
                    }

                    using affine<Q,d>::transformationMatrix;

                protected:
                    typename euclidian::space<Q,d>::vector from;
            };
        };
    };
};

#endif