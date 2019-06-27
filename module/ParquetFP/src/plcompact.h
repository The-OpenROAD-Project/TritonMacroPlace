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


#ifndef PLCOMPACT_H
#define PLCOMPACT_H

#include "basepacking.h"

#include <iostream>

// --------------------------------------------------------
// back-end database to determine how much a block can/has to move
// *** NOT INTENDED TO BE USED DIRECTLY, USE ShiftLegalizer INSTEAD ***
class ShiftBlock
{
public:
   ShiftBlock(const uofm::vector<float>& xloc,
              const uofm::vector<float>& yloc,
              const uofm::vector<float>& widths,
              const uofm::vector<float>& heights,
              float left_bound,
              float right_bound,
              float top_bound,
              float bottom_bound);

   class ShiftInfo
   {
   public:
      float shiftRangeMin; 
      float shiftRangeMax;      
      float overlapMin;
      float overlapMax;
   };
   void operator ()(int currBlk,
                    uofm::vector<ShiftInfo>& shiftinfo) const; // 4-elt uofm::vector

   enum Directions {NORTH, EAST, SOUTH, WEST, DIR_NUM};
   static const float Infty;
   static const float Neg_Infty;
   static const int Undefined;

private:
   int _blocknum;
   uofm::vector<float> _xStart;
   uofm::vector<float> _xEnd;
   uofm::vector<float> _yStart;
   uofm::vector<float> _yEnd;

   float _epsilon;
};
// --------------------------------------------------------
// front-end legalizer to be used
class ShiftLegalizer
{
public:
   ShiftLegalizer(const uofm::vector<float>& n_xloc,
                  const uofm::vector<float>& n_yloc,
                  const uofm::vector<float>& n_widths,
                  const uofm::vector<float>& n_heights,
                  float left_bound,
                  float right_bound,
                  float top_bound,
                  float bottom_bound);

   enum AlgoType {NAIVE};
   
   bool legalizeAll(AlgoType algo,
                    const uofm::vector<int>& checkBlks,
                    uofm::vector<int>& badBlks);   

   bool naiveLegalize(const uofm::vector<int>& checkBlks,
                      uofm::vector<int>& badBlks); // "t" ~ badBlks.empty() 
   bool legalizeBlock(int currBlk); // "t" ~ "currBlk" not overlap

   void putBlockIntoCore(int currBlk);

   inline const uofm::vector<float>& xloc() const;
   inline const uofm::vector<float>& yloc() const;
   inline const uofm::vector<float>& widths() const;
   inline const uofm::vector<float>& heights() const;

   inline float leftBound() const;
   inline float rightBound() const;
   inline float topBound() const;
   inline float bottomBound() const;

   static const float Infty;
   static const int Undefined;

   class ShiftRotateDecision
   {
   public:
      inline ShiftRotateDecision();
      
      bool rotate;
      int shiftDir;
      float shiftExtent;
   };
   bool shiftDecider(ShiftBlock::Directions currDir,
                     const uofm::vector<ShiftBlock::ShiftInfo>& shiftinfo,
                     ShiftRotateDecision& decision) const;
   bool rotateDecider(int currBlk,
                      const uofm::vector<ShiftBlock::ShiftInfo>& shiftinfo,
                      ShiftRotateDecision& decision) const;                            
   
private:
   int _blocknum;
   uofm::vector<float> _xloc;
   uofm::vector<float> _yloc;
   uofm::vector<float> _widths;
   uofm::vector<float> _heights;
   float _epsilon;

   const float _leftBound;
   const float _rightBound;
   const float _topBound;
   const float _bottomBound;

   void adjustBlock(int currBlk, const ShiftRotateDecision& decision);
};
void OutputShiftInfo(std::ostream& outs,
                     const uofm::vector<ShiftBlock::ShiftInfo>& shiftinfo);
// --------------------------------------------------------

// ===============
// IMPLEMENTATIONS
// ===============
const uofm::vector<float>& ShiftLegalizer::xloc() const
{   return _xloc; }
// --------------------------------------------------------
const uofm::vector<float>& ShiftLegalizer::yloc() const
{   return _yloc; }
// --------------------------------------------------------
const uofm::vector<float>& ShiftLegalizer::widths() const
{   return _widths; }
// --------------------------------------------------------
const uofm::vector<float>& ShiftLegalizer::heights() const
{   return _heights; }
// --------------------------------------------------------
float ShiftLegalizer::leftBound() const
{   return _leftBound; }
// --------------------------------------------------------
float ShiftLegalizer::rightBound() const
{   return _rightBound; }
// --------------------------------------------------------
float ShiftLegalizer::topBound() const
{   return _topBound; }
// --------------------------------------------------------
float ShiftLegalizer::bottomBound() const
{   return _bottomBound; }
// --------------------------------------------------------
ShiftLegalizer::ShiftRotateDecision::ShiftRotateDecision()
   : rotate(false),
     shiftDir(Undefined),
     shiftExtent(Infty)
{}
// --------------------------------------------------------

#endif
