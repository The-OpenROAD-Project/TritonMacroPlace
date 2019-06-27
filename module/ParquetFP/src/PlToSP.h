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


#ifndef PLTOSP_H
#define PLTOSP_H

#include <ABKCommon/uofm_alloc.h>
#include <fstream>
#include <algorithm>
#include <cmath>
#include <cstdlib>

namespace parquetfp
{
   enum PL2SP_ALGO{NAIVE_ALGO, TCG_ALGO};

   class Pl2SP
   {
   private:
      uofm::vector<float> _xloc;
      uofm::vector<float> _yloc;
      uofm::vector<float> _widths;
      uofm::vector<float> _heights;

      uofm::vector<unsigned> _XX;
      uofm::vector<unsigned> _YY;

      int _cnt;
   public:
      Pl2SP(uofm::vector<float>& xloc, uofm::vector<float>& yloc, uofm::vector<float>& widths,
            uofm::vector<float>& heights, PL2SP_ALGO whichAlgo);

      ~Pl2SP() {}
    
      void naiveAlgo(void);
      void TCGAlgo(void);

      //Floyd Marshal to find TCG
      void TCG_FM(uofm::vector< uofm::vector <bool> >& TCGMatrixHoriz, 
                  uofm::vector< uofm::vector <bool> >& TCGMatrixVert);

      //DP to find TCG
      void TCG_DP(uofm::vector< uofm::vector <bool> >& TCGMatrixHoriz, 
                  uofm::vector< uofm::vector <bool> >& TCGMatrixVert);
      void TCGDfs(uofm::vector< uofm::vector <bool> >& TCGMatrix, 
                  uofm::vector< uofm::vector <bool> >& adjMatrix, int v, 
                  uofm::vector<int>& pre);

      const uofm::vector<unsigned>& getXSP(void) const
         { return _XX; }
     
      const uofm::vector<unsigned>& getYSP(void) const
         { return _YY; }

      void print(void) const;

   };

   struct RowElem
   {
      unsigned index;
      float xloc;
   };


   struct less_mag 
   {
      bool operator()(const RowElem &elem1, const RowElem &elem2) 
         { return(elem1.xloc < elem2.xloc); }
   };

   class SPXRelation
   {
      const uofm::vector< uofm::vector<bool> >& TCGMatrixHoriz;
      const uofm::vector< uofm::vector<bool> >& TCGMatrixVert;

   public:
      SPXRelation(const uofm::vector< uofm::vector<bool> >& TCGMatrixHorizIP, 
                  const uofm::vector< uofm::vector<bool> >& TCGMatrixVertIP) : 
         TCGMatrixHoriz(TCGMatrixHorizIP), TCGMatrixVert(TCGMatrixVertIP)
         {}

      bool operator()(const unsigned i, const unsigned j) const
         {
            if(i == j)
            {
              return false;
            }
            else if(TCGMatrixHoriz[i][j])
            {
               return true;
            }
            else if(TCGMatrixHoriz[j][i])
            {
               return false;
            }
            else if(TCGMatrixVert[j][i])
            {
               return true;
            }
            else if(TCGMatrixVert[i][j])
            {
               return false;
            }
            else
            {
               //cout<<"ERROR IN PL2SP SPX "<<i<<"\t"<<j<<endl;
               if(i<j)
               {
                  return true;
               }
               else
               {
                  return false;
               }
            }
         }
   };

   class SPYRelation
   {
      const uofm::vector< uofm::vector<bool> >& TCGMatrixHoriz;
      const uofm::vector< uofm::vector<bool> >& TCGMatrixVert;

   public:
      SPYRelation(const uofm::vector< uofm::vector<bool> >& TCGMatrixHorizIP, 
                  const uofm::vector< uofm::vector<bool> >& TCGMatrixVertIP) : 
         TCGMatrixHoriz(TCGMatrixHorizIP), TCGMatrixVert(TCGMatrixVertIP)
         {}
      bool operator()(const unsigned i, const unsigned j) const
         {
            if(i == j)
            {
              return false;
            }
            if(TCGMatrixHoriz[i][j])
               return true;
            else if(TCGMatrixHoriz[j][i])
               return false;
            else if(TCGMatrixVert[j][i])
               return false;
            else if(TCGMatrixVert[i][j])
               return true;
            else
            {
               //cout<<"ERROR IN PL2SP SPY "<<i<<"\t"<<j<<endl;
               if(i<j)
                  return true;
               else
                  return false;
            }
         }
   };
}
//using namespace parquetfp;

#endif 
