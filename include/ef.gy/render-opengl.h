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

#if !defined(EF_GY_RENDER_OPENGL_H)
#define EF_GY_RENDER_OPENGL_H

#include <ef.gy/euclidian.h>
#include <ef.gy/projection.h>
#include <vector>
#include <map>

#undef GL3D
#define GL3D

//#define GLVA

namespace efgy
{
    namespace render
    {
        // Attribute index.
        enum
        {
            attributePosition,
            attributeNormal,
            attributeColour
        } openGLShaderAttribute;

        template<typename Q, unsigned int d>
        class opengl
        {
            public:
                opengl
                    (const geometry::transformation<Q,d> &pTransformation,
                     const geometry::projection<Q,d> &pProjection,
                     opengl<Q,d-1> &pLowerRenderer)
                    : transformation(pTransformation), projection(pProjection), lowerRenderer(pLowerRenderer)
                    {}

                void frameStart (void)
                {
                    combined = transformation * projection;
                    lowerRenderer.frameStart();
                };
                void frameEnd (void) const { lowerRenderer.frameEnd(); };
                void pushLines (void) const { lowerRenderer.pushLines(); };
                void pushFaces (void) const { lowerRenderer.pushFaces(); };

                void drawLine
                    (const typename geometry::euclidian::space<Q,d>::vector &pA,
                     const typename geometry::euclidian::space<Q,d>::vector &pB) const;

                template<unsigned int q>
                void drawFace
                    (const math::tuple<q, typename geometry::euclidian::space<Q,d>::vector> &pV) const;

                void reset (void) const { lowerRenderer.reset(); }
                const bool isPrepared (void) const { return lowerRenderer.isPrepared(); }
                bool setColour (float red, float green, float blue, float alpha)
                {
                    return lowerRenderer.setColour(red,green,blue,alpha);
                }

            protected:
                const geometry::transformation<Q,d> &transformation;
                const geometry::projection<Q,d> &projection;
                geometry::transformation<Q,d> combined;
                opengl<Q,d-1> &lowerRenderer;
        };

#if defined(GL3D)
        template<typename Q>
        class opengl<Q,3>
        {
            public:
                opengl
                    (const geometry::transformation<Q,3> &pTransformation,
                     const geometry::projection<Q,3> &pProjection,
                     const opengl<Q,2> &)
                    : transformation(pTransformation), projection(pProjection),
                      haveBuffers(false), prepared(false)
                    {
                    }

                ~opengl (void)
                    {
                        if (haveBuffers)
                        {
                            glDeleteBuffers(1, &vertexbuffer);
                            glDeleteBuffers(1, &elementbuffer);
                            glDeleteBuffers(1, &linebuffer);
                        }
                    }

                void frameStart (void)
                {
#if !defined(GLVA)
                    glMatrixMode(GL_MODELVIEW);
                    GLfloat mat[16] =
                        { GLfloat(transformation.transformationMatrix.data[0][0]),
                          GLfloat(transformation.transformationMatrix.data[0][1]),
                          GLfloat(transformation.transformationMatrix.data[0][2]),
                          GLfloat(transformation.transformationMatrix.data[0][3]),
                          GLfloat(transformation.transformationMatrix.data[1][0]),
                          GLfloat(transformation.transformationMatrix.data[1][1]),
                          GLfloat(transformation.transformationMatrix.data[1][2]),
                          GLfloat(transformation.transformationMatrix.data[1][3]),
                          GLfloat(transformation.transformationMatrix.data[2][0]),
                          GLfloat(transformation.transformationMatrix.data[2][1]),
                          GLfloat(transformation.transformationMatrix.data[2][2]),
                          GLfloat(transformation.transformationMatrix.data[2][3]),
                          GLfloat(transformation.transformationMatrix.data[3][0]),
                          GLfloat(transformation.transformationMatrix.data[3][1]),
                          GLfloat(transformation.transformationMatrix.data[3][2]),
                          GLfloat(transformation.transformationMatrix.data[3][3]) };
                    glLoadMatrixf(mat);
                    glMatrixMode(GL_PROJECTION);
                    /*
                    GLfloat matp[16] =
                        { GLfloat(projection.transformationMatrix.data[0][0]),
                          GLfloat(projection.transformationMatrix.data[0][1]),
                          GLfloat(projection.transformationMatrix.data[0][2]),
                          GLfloat(projection.transformationMatrix.data[0][3]),
                          GLfloat(projection.transformationMatrix.data[1][0]),
                          GLfloat(projection.transformationMatrix.data[1][1]),
                          GLfloat(projection.transformationMatrix.data[1][2]),
                          GLfloat(projection.transformationMatrix.data[1][3]),
                          GLfloat(projection.transformationMatrix.data[2][0]),
                          GLfloat(projection.transformationMatrix.data[2][1]),
                          GLfloat(projection.transformationMatrix.data[2][2]),
                          GLfloat(projection.transformationMatrix.data[2][3]),
                          GLfloat(projection.transformationMatrix.data[3][0]),
                          GLfloat(projection.transformationMatrix.data[3][1]),
                          GLfloat(projection.transformationMatrix.data[3][2]),
                          GLfloat(projection.transformationMatrix.data[3][3]) };
                    glLoadMatrixf(matp);
                     */
                    glLoadIdentity();
#endif
                    
                    if(!haveBuffers)
                    {
                        haveBuffers = true;

#if defined(GLVA)
                        glGenVertexArrays(1, &VertexArrayID);
#endif
                        glGenBuffers(1, &vertexbuffer);
                        glGenBuffers(1, &elementbuffer);
                        glGenBuffers(1, &linebuffer);
                    }
                    /*
                    if (prepared)
                    {
                        prepared = false;
                    }
                     */
                };
                void frameEnd (void)
                {
                    if (!prepared)
                    {
                        prepared = true;

                        //std::cerr << "cn:" << vertices.size() << "=" << (vertices.size()/6) << ":" << triindices.size() << "=" << (triindices.size()/3)<< ":" << lineindices.size() << "=" << (lineindices.size()/2)<< "\n";

#if defined(GLVA)
                        glGenVertexArrays(1, &VertexArrayID);
                        glBindVertexArray(VertexArrayID);
#endif
                        glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
                        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), &vertices[0], GL_STATIC_DRAW);
                        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
                        glBufferData(GL_ELEMENT_ARRAY_BUFFER, triindices.size() * sizeof(unsigned int), &triindices[0], GL_STATIC_DRAW);
                        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, linebuffer);
                        glBufferData(GL_ELEMENT_ARRAY_BUFFER, lineindices.size() * sizeof(unsigned int), &lineindices[0], GL_STATIC_DRAW);
#if defined(GLVA)
                        glEnableVertexAttribArray(attributePosition);
                        glVertexAttribPointer(attributePosition, 3, GL_FLOAT, GL_FALSE, 10*sizeof(GLfloat), 0);
                        glEnableVertexAttribArray(attributeNormal);
                        glVertexAttribPointer(attributeNormal, 3, GL_FLOAT, GL_FALSE, 10*sizeof(GLfloat), (void*)(3*sizeof(GLfloat)));
                        glEnableVertexAttribArray(attributeColour);
                        glVertexAttribPointer(attributeColour, 4, GL_FLOAT, GL_FALSE, 10*sizeof(GLfloat), (void*)(6*sizeof(GLfloat)));

                        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
                        glBindBuffer(GL_ARRAY_BUFFER, 0);
                        glBindVertexArray(0);
#endif

                        tindices = GLsizei(triindices.size());
                        lindices = GLsizei(lineindices.size());

                        vertices.clear();
                        vertexmap.clear();
                        triindices.clear();
                        lineindices.clear();
                        indices = 0;
                    }
                };

                void pushLines (void) const
                {
                    if (prepared)
                    {
#if defined(GLVA)
                        glBindVertexArray(VertexArrayID);
                        
                        glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
                        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, linebuffer);
                        glDrawArrays(GL_LINES, 0, lindices);
                        
                        glBindBuffer(GL_ARRAY_BUFFER, 0);
                        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
                        glBindVertexArray(0);
#else
                        glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
                        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, linebuffer);
                        glEnableClientState(GL_VERTEX_ARRAY);
                        glEnableClientState(GL_NORMAL_ARRAY);
                        glVertexPointer(3, GL_FLOAT, 6*sizeof(GLfloat), 0);
                        glNormalPointer(GL_FLOAT, 6*sizeof(GLfloat), (void*)(3*sizeof(GLfloat)));
                        glDrawElements (GL_LINES, lindices, GL_UNSIGNED_INT, 0);
                        
                        glDisableClientState(GL_NORMAL_ARRAY);
                        glDisableClientState(GL_VERTEX_ARRAY);
                        
                        glBindBuffer(GL_ARRAY_BUFFER, 0);
                        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
#endif
                    }
                }

                void pushFaces (void) const
                {
                    if (prepared)
                    {
#if defined(GLVA)
                        glBindVertexArray(VertexArrayID);

                        glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
                        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
                        glDrawArrays(GL_TRIANGLES, lindices, tindices);
                        
                        glBindBuffer(GL_ARRAY_BUFFER, 0);
                        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
                        glBindVertexArray(0);
#else
                        glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
                        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
                        glEnableClientState(GL_VERTEX_ARRAY);
                        glEnableClientState(GL_NORMAL_ARRAY);
                        glVertexPointer(3, GL_FLOAT, 6*sizeof(GLfloat), 0);
                        glNormalPointer(GL_FLOAT, 6*sizeof(GLfloat), (void*)(3*sizeof(GLfloat)));
                        glDrawElements (GL_TRIANGLES, tindices, GL_UNSIGNED_INT, 0);

                        glDisableClientState(GL_NORMAL_ARRAY);
                        glDisableClientState(GL_VERTEX_ARRAY);

                        glBindBuffer(GL_ARRAY_BUFFER, 0);
                        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
#endif
                    }
                }

                void drawLine
                    (const typename geometry::euclidian::space<Q,3>::vector &pA,
                     const typename geometry::euclidian::space<Q,3>::vector &pB);

                template<unsigned int q>
                void drawFace
                    (const math::tuple<q, typename geometry::euclidian::space<Q,3>::vector> &pV);

                void reset (void) { prepared = false; }
                const bool isPrepared (void) const { return prepared; }

                unsigned int addVertex
                    (const GLfloat &x, const GLfloat &y, const GLfloat &z,
                     const GLfloat &nx = 0.f, const GLfloat &ny = 0.f, const GLfloat &nz = 0.f)
                {
#if !defined(GLVA)
                    std::vector<GLfloat> key (6);
                    key[0] = x;
                    key[1] = y;
                    key[2] = z;
                    key[3] = nx;
                    key[4] = ny;
                    key[5] = nz;
                    
                    std::map<std::vector<GLfloat>,unsigned int>::iterator it = vertexmap.find(key);
                    if (it != vertexmap.end())
                    {
                        return it->second;
                    }
#endif

                    vertices.push_back(x);
                    vertices.push_back(y);
                    vertices.push_back(z);

                    vertices.push_back(nx);
                    vertices.push_back(ny);
                    vertices.push_back(nz);

#if defined(GLVA)
                    vertices.push_back(red);
                    vertices.push_back(green);
                    vertices.push_back(blue);
                    vertices.push_back(alpha);
#endif

                    unsigned int rv = indices;

                    indices++;

#if !defined(GLVA)
                    vertexmap[key] = rv;
#endif

                    return rv;
                }

                bool setColour (float pRed, float pGreen, float pBlue, float pAlpha)
                {
#if defined (GLVA)
                    red   = pRed;
                    green = pGreen;
                    blue  = pBlue;
                    alpha = pAlpha;
#else
                    glColor4f(pRed, pGreen, pBlue, pAlpha);
#endif
                    return true;
                }

            protected:
                const geometry::transformation<Q,3> &transformation;
                const geometry::projection<Q,3> &projection;
                std::vector<GLfloat> vertices;
                std::map<std::vector<GLfloat>,unsigned int> vertexmap;
                std::vector<unsigned int> triindices;
                std::vector<unsigned int> lineindices;
                unsigned int indices;
                GLsizei tindices;
                GLsizei lindices;
                bool haveBuffers;
                bool prepared;
                GLuint VertexArrayID;
                GLuint vertexbuffer;
                GLuint elementbuffer;
                GLuint linebuffer;
#if defined (GLVA)
                GLfloat red;
                GLfloat green;
                GLfloat blue;
                GLfloat alpha;
#endif
        };
#endif

        template<typename Q>
        class opengl<Q,2>
        {
            public:
                opengl
                    (const geometry::transformation<Q,2> &pTransformation)
                    : transformation(pTransformation)
                    {}

                void frameStart (void) const {};
                void frameEnd (void) const {};
                void pushLines (void) const {};
                void pushFaces (void) const {};

                void drawLine
                    (const typename geometry::euclidian::space<Q,2>::vector &pA,
                     const typename geometry::euclidian::space<Q,2>::vector &pB) const;

                template<unsigned int q>
                void drawFace
                    (const math::tuple<q, typename geometry::euclidian::space<Q,2>::vector> &pV) const;

                void reset (void) {}
                const bool isPrepared (void) const { return false; }
                bool setColour (float red, float green, float blue, float alpha) const { return false; }

            protected:
                const geometry::transformation<Q,2> &transformation;
        };

        template<typename Q, unsigned int d>
        void opengl<Q,d>::drawLine
            (const typename geometry::euclidian::space<Q,d>::vector &pA,
             const typename geometry::euclidian::space<Q,d>::vector &pB) const
        {
            if (isPrepared()) return;
            
            typename geometry::euclidian::space<Q,d-1>::vector A = combined.project(pA);
            typename geometry::euclidian::space<Q,d-1>::vector B = combined.project(pB);

            lowerRenderer.drawLine(A, B);
        }

        template<typename Q, unsigned int d>
        template<unsigned int q>
        void opengl<Q,d>::drawFace
            (const math::tuple<q, typename geometry::euclidian::space<Q,d>::vector> &pV) const
        {
            if (isPrepared()) return;

            math::tuple<q, typename geometry::euclidian::space<Q,d-1>::vector> V;

            for (unsigned int i = 0; i < q; i++)
            {
                V.data[i] = combined.project(pV.data[i]);
            }

            lowerRenderer.drawFace(V);
        }

#if defined(GL3D)
        template<typename Q>
        void opengl<Q,3>::drawLine
            (const typename geometry::euclidian::space<Q,3>::vector &A,
             const typename geometry::euclidian::space<Q,3>::vector &B)
        {
            if (isPrepared()) return;

            lineindices.push_back(addVertex(GLfloat(A.data[0]), GLfloat(A.data[1]), GLfloat(A.data[2])));
            lineindices.push_back(addVertex(GLfloat(B.data[0]), GLfloat(B.data[1]), GLfloat(B.data[2])));
        }

        template<typename Q>
        template<unsigned int q>
        void opengl<Q,3>::drawFace
            (const math::tuple<q, typename geometry::euclidian::space<Q,3>::vector> &pV)
        {
            if (isPrepared()) return;

            typename geometry::euclidian::space<Q,3>::vector R
                = geometry::euclidian::normalise<Q,3>
                    (geometry::euclidian::crossProduct<Q>
                        (pV.data[1] - pV.data[0], pV.data[2] - pV.data[0]));

            typename geometry::euclidian::space<Q,3>::vector RN
                = geometry::euclidian::normalise<Q,3>
                    (geometry::euclidian::crossProduct<Q>
                        (pV.data[2] - pV.data[0], pV.data[1] - pV.data[0]));

            triindices.push_back(addVertex(GLfloat(pV.data[0].data[0]), GLfloat(pV.data[0].data[1]), GLfloat(pV.data[0].data[2]),
                                           GLfloat(R.data[0]), GLfloat(R.data[1]), GLfloat(R.data[2])));
            triindices.push_back(addVertex(GLfloat(pV.data[1].data[0]), GLfloat(pV.data[1].data[1]), GLfloat(pV.data[1].data[2]),
                                           GLfloat(R.data[0]), GLfloat(R.data[1]), GLfloat(R.data[2])));
            triindices.push_back(addVertex(GLfloat(pV.data[2].data[0]), GLfloat(pV.data[2].data[1]), GLfloat(pV.data[2].data[2]),
                                           GLfloat(R.data[0]), GLfloat(R.data[1]), GLfloat(R.data[2])));

            triindices.push_back(addVertex(GLfloat(pV.data[2].data[0]), GLfloat(pV.data[2].data[1]), GLfloat(pV.data[2].data[2]),
                                           GLfloat(RN.data[0]), GLfloat(RN.data[1]), GLfloat(RN.data[2])));
            triindices.push_back(addVertex(GLfloat(pV.data[1].data[0]), GLfloat(pV.data[1].data[1]), GLfloat(pV.data[1].data[2]),
                                           GLfloat(RN.data[0]), GLfloat(RN.data[1]), GLfloat(RN.data[2])));
            triindices.push_back(addVertex(GLfloat(pV.data[0].data[0]), GLfloat(pV.data[0].data[1]), GLfloat(pV.data[0].data[2]),
                                           GLfloat(RN.data[0]), GLfloat(RN.data[1]), GLfloat(RN.data[2])));

            for (unsigned int j = 3; j < q; j++)
            {
                triindices.push_back(addVertex(GLfloat(pV.data[0].data[0]), GLfloat(pV.data[0].data[1]), GLfloat(pV.data[0].data[2]),
                                               GLfloat(R.data[0]), GLfloat(R.data[1]), GLfloat(R.data[2])));
                triindices.push_back(addVertex(GLfloat(pV.data[(j-1)].data[0]), GLfloat(pV.data[(j-1)].data[1]), GLfloat(pV.data[(j-1)].data[2]),
                                               GLfloat(R.data[0]), GLfloat(R.data[1]), GLfloat(R.data[2])));
                triindices.push_back(addVertex(GLfloat(pV.data[j].data[0]), GLfloat(pV.data[j].data[1]), GLfloat(pV.data[j].data[2]),
                                               GLfloat(R.data[0]), GLfloat(R.data[1]), GLfloat(R.data[2])));

                triindices.push_back(addVertex(GLfloat(pV.data[j].data[0]), GLfloat(pV.data[j].data[1]), GLfloat(pV.data[j].data[2]),
                                               GLfloat(RN.data[0]), GLfloat(RN.data[1]), GLfloat(RN.data[2])));
                triindices.push_back(addVertex(GLfloat(pV.data[(j-1)].data[0]), GLfloat(pV.data[(j-1)].data[1]), GLfloat(pV.data[(j-1)].data[2]),
                                               GLfloat(RN.data[0]), GLfloat(RN.data[1]), GLfloat(RN.data[2])));
                triindices.push_back(addVertex(GLfloat(pV.data[0].data[0]), GLfloat(pV.data[0].data[1]), GLfloat(pV.data[0].data[2]),
                                               GLfloat(RN.data[0]), GLfloat(RN.data[1]), GLfloat(RN.data[2])));
            }
        }
#endif

        template<typename Q>
        void opengl<Q,2>::drawLine
            (const typename geometry::euclidian::space<Q,2>::vector &pA,
             const typename geometry::euclidian::space<Q,2>::vector &pB) const
        {
            const typename geometry::euclidian::space<Q,3>::vector A = transformation * pA;
            const typename geometry::euclidian::space<Q,3>::vector B = transformation * pB;

            const GLfloat vertices[6] =
                { GLfloat(A.data[0]), GLfloat(A.data[1]), 0,
                  GLfloat(B.data[0]), GLfloat(B.data[1]), 0 };

            glNormalPointer(GL_FLOAT, 0, vertices);
            glVertexPointer(3, GL_FLOAT, 0, vertices);
            glDrawArrays(GL_LINES, 0, 2);
        }

        template<typename Q>
        template<unsigned int q>
        void opengl<Q,2>::drawFace
            (const math::tuple<q, typename geometry::euclidian::space<Q,2>::vector> &pV) const
        {
            GLfloat vertices[(q*3)];
            for (unsigned int i = 0; i < q; i++)
            {
                const typename geometry::euclidian::space<Q,3>::vector V = transformation * pV.data[i];

                vertices[(i*3)+0] = V.data[0];
                vertices[(i*3)+1] = V.data[1];
                vertices[(i*3)+2] = 0;
            }
            glNormalPointer(GL_FLOAT, 0, vertices);
            glVertexPointer(3, GL_FLOAT, 0, vertices);
            glDrawArrays(GL_TRIANGLE_FAN, 0, q);
        }
    };
};

#endif
