/**************************************************************************
***    
*** Copyright (c) 2000-2006 Regents of the University of Michigan,
***               Saurabh N. Adya, Hayward Chan, Jarrod A. Roy
***               and Igor L. Markov
***
***  Contact author(s): sadya@umich.edu, imarkov@umich.edu
***  Original Affiliation:   University of Michigan, EECS Dept.
***                          Ann Arbor, MI 48109-2122 USA
***
***  Permission is hereby granted, free of charge, to any person obtaining 
***  a copy of this software and associated documentation files (the
***  "Software"), to deal in the Software without restriction, including
***  without limitation 
***  the rights to use, copy, modify, merge, publish, distribute, sublicense, 
***  and/or sell copies of the Software, and to permit persons to whom the 
***  Software is furnished to do so, subject to the following conditions:
***
***  The above copyright notice and this permission notice shall be included
***  in all copies or substantial portions of the Software.
***
*** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
*** EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
*** OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
*** IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
*** CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT
*** OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
*** THE USE OR OTHER DEALINGS IN THE SOFTWARE.
***
***
***************************************************************************/


#ifndef SPEVAL_H
#define SPEVAL_H

#include <fstream>
#include <iostream>
#include <map>
#include <algorithm>
#include <cmath>
#include <cstdlib>

namespace parquetfp
{
   class SPeval
   {
   private:
      //buffers go in here
      uofm::vector<unsigned> _match;
      uofm::vector<unsigned> _reverseXX;
      uofm::vector<unsigned> _reverseYY;
      uofm::vector<float> _LL;      
      uofm::vector<float> _heights;
      uofm::vector<float> _widths;
      uofm::vector<float> _xlocRev;
      uofm::vector<float> _ylocRev;
      std::map<unsigned , float> _BST;  //for the O(nlogn) algo

      uofm::vector< uofm::vector<bool> > _TCGMatrixHoriz;
      uofm::vector< uofm::vector<bool> > _TCGMatrixVert;

      float _lcsCompute(const uofm::vector<unsigned>& X,
                        const uofm::vector<unsigned>& Y,
                        const uofm::vector<float>& weights,
                        uofm::vector<unsigned>& match,
                        uofm::vector<float>& P,
                        uofm::vector<float>& L
         );

      float _lcsReverseCompute(const uofm::vector<unsigned>& X,
                               const uofm::vector<unsigned>& Y,
                               const uofm::vector<float>& weights,
                               uofm::vector<unsigned>& match,
                               uofm::vector<float>& P,
                               uofm::vector<float>& L
         );
  
      float _lcsComputeCompact(const uofm::vector<unsigned>& X,
                               const uofm::vector<unsigned>& Y,
                               const uofm::vector<float>& weights,
                               uofm::vector<unsigned>& match,
                               uofm::vector<float>& P,
                               uofm::vector<float>& L,
                               uofm::vector<float>& oppLocs,
                               uofm::vector<float>& oppWeights
         );
  
      //fast are for the O(nlog n) algo
      float _findBST(unsigned index); //see the paper for definitions
      void _discardNodesBST(unsigned index, float length);

      float _lcsComputeFast(const uofm::vector<unsigned>& X,
                             const uofm::vector<unsigned>& Y,
                             const uofm::vector<float>& weights,
                             uofm::vector<unsigned>& match,
                             uofm::vector<float>& P,
                             uofm::vector<float>& L
         );


      bool _TCGMatrixInitialized;
      void _initializeTCGMatrix(unsigned size);
      bool _paramUseFastSP;

   public:
      uofm::vector<float> xloc;
      uofm::vector<float> yloc;
      uofm::vector<float> xlocRev;
      uofm::vector<float> ylocRev;
      float xSize;
      float ySize;
      uofm::vector<float> xSlacks;
      uofm::vector<float> ySlacks;


      SPeval(const uofm::vector<float>& heights,
             const uofm::vector<float>& widths,
             bool paramUseFastSP);

  
      void evaluate(const uofm::vector<unsigned>& X, const uofm::vector<unsigned>& Y,
                    bool leftpack, bool botpack);      
      void evalSlacks(const uofm::vector<unsigned>& X, const uofm::vector<unsigned>& Y);
      void evaluateCompact(const uofm::vector<unsigned>& X,
                           const uofm::vector<unsigned>& Y, 
                           bool whichDir, bool leftpack, bool botpack);

      //following are for evaluating with the O(nlog n) scheme
      void evaluateFast(const uofm::vector<unsigned>& X, const uofm::vector<unsigned>& Y,
                        bool leftpack, bool botpack);
      void evalSlacksFast(const uofm::vector<unsigned>& X, const uofm::vector<unsigned>& Y);
   private:

      float xEval(const uofm::vector<unsigned>& X, const uofm::vector<unsigned>& Y,
                  bool leftpack, uofm::vector<float>& xcoords);
      float yEval(const uofm::vector<unsigned>& X, const uofm::vector<unsigned>& Y,
                  bool botpack, uofm::vector<float>& ycoords);
      float xEvalCompact(const uofm::vector<unsigned>& X, const uofm::vector<unsigned>& Y,
                         bool leftpack, uofm::vector<float>& xcoords, uofm::vector<float>& ycoords);
      float yEvalCompact(const uofm::vector<unsigned>& X, const uofm::vector<unsigned>& Y,
                         bool botpack, uofm::vector<float>& xcoords, uofm::vector<float>& ycoords);
      void computeConstraintGraphs(const uofm::vector<unsigned>& X, const uofm::vector<unsigned>& Y);
      void removeRedundantConstraints(const uofm::vector<unsigned>& X, const uofm::vector<unsigned>& Y, bool knownDir);
      void computeSPFromCG(uofm::vector<unsigned>& X, uofm::vector<unsigned>& Y);

      //following are for evaluating with the O(nlog n) scheme
      float xEvalFast(const uofm::vector<unsigned>& X, const uofm::vector<unsigned>& Y,
                      bool leftpack, uofm::vector<float>& xcoords);
      float yEvalFast(const uofm::vector<unsigned>& X, const uofm::vector<unsigned>& Y,
                      bool botpack,  uofm::vector<float>& ycoords);

   public:
      //miscelleneous functions
      void changeWidths(const uofm::vector<float>& widths);
      void changeHeights(const uofm::vector<float>& heights);
      void changeNodeWidth(const unsigned index, float width);
      void changeNodeHeight(const unsigned index, float height);
      void changeOrient(unsigned index);
   };
}
//using namespace parquetfp;

#endif
