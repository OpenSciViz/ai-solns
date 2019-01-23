#!/usr/bin/env python

# import numpy as np
import os, sys

try:
  from test_data import clay_init, clay_readfile, clay_writefile, clay_print
except:
  print("overflow import of test_data module failed")
  sys.exit(1)

_flowcnt = 0
_wetcnt = 0
_dims = [14, 14]

def flow_down(tiles, x0=0, y0=0, dim=14):
  """
  Return index of first clay tile found below current/origin tile and if none are found, return -1.
  Downward flow is possible (not blocked by clay), if -1 is returned, or an index > (y0 + 1) is
  returned, otherwise downward flow is blocked. Set wet tiles to '|' along path to
  clay or if no clay all should be set to '|'. If flow is blocked up then upper '|' may get 
  reset to '~' once steady-state / oveflow is achieved. 
  """
  global _flowcnt
  yclay = -1
  for y in range(1+y0, dim-1):
    if(tiles[y][x0] == '#'):
      yclay = y
  
  if( yclay < 0 ):
    # set all tiles below to '|' and return index of deepest '|'
    deepest = dim-1
    for y in range(y0, dim-y0-1):
      tiles[y][x0] = '|'
      _flowcnt += 1
    return deepest

  deepest = yclay - 1
  # set the deepest non-clay tile those above to '|'
  for y in range(deepest, 0, -1):
    tiles[y][x0] = '|' 
    _flowcnt += 1

  print("flow_down> _flowcnt:", _flowcnt)
  return deepest

def flow_right(tiles, x0=0, y0=0, dim=14):
  """
  Return index of first clay tile found right of current/origin tile and if none are found, return -1.
  Rightward flow is possible (not blocked by clay), if -1 is returned, or an index > x+1 is returned,
  otherwise rightward flow is blocked. Set wet tiles to either '|' along path to clay or if no clay all
  should be set to '|'. If flow is blocked up then '|' may get reset to '~' once steady-stete is achieved. 
  """
  global _flowcnt
  for xr in range(x0+1, dim-1):
    if(tiles[y0][xr] == '#'):
      return xr
    else:
      tiles[y0][xr] = '|' ; _flowcnt += 1 # flowing right
    if( tiles[y0+1][xr] == '.' ): # flow down -- stop flowing right and try down
      deepest = flow_down(tiles, xr, y0+1, dim)
      return [xr, deepest]

  # if we get here no clay tile found right of x0, y0
  print("flow_right> _flowcnt:", _flowcnt)
  return [-1, y0]
 
def flow_left(tiles, x0=0, y0=0, dim=14):
  """
  Return index of first clay tile found left of current/origin tile and if none are found, return -1.
  Leftward flow is possible (not blocked by clay), if -1 is returned, or an index < x-1 is returned,
  otherwise leftwardward flow is blocked. Set wet tiles to either '|' or '+' along path to
  clay or if no clay all should be set to '|'. If flow is blocked up then upper '|' may get 
  reset to '~' once steady-stete is achieved. 
  """
  global _flowcnt
  for xl in range(x0-1, 0, -1):
    if(tiles[y0][xl] == '#'):
      return [xl, y0]
    else:
      tiles[y0][xl] = '|' ; _flowcnt += 1 # flowing left
      if( tiles[y0+1][xl] == '.' ): # stop flowing left and head down
        tiles[y0][xl] = '|' 
        deepest = flow_down(tiles,xl,y0+1,dim)
        return [xl, deepest]

  # if we get here no clay tile found left of x0, y0
  print("flow_left> _flowcnt:", _flowcnt)
  return [-1, y0]
  
def flow(tiles, xsrc, y0, dim=14):
  """
  Inspect tiles to left and right of current (x0, y0) and alse directly below
  to left and right of (x0, y0+1). If symmetric, flow should go both ways and
  continue down of fill up. Otherwise, flow proceeds down nearest open (non-clay)
  path, etc. 
  """
  global _flowcnt
  xl = xr = xsrc

  yd = flow_down(tiles, xsrc, y0, dim)
  xld = flow_left(tiles, xl, y0, dim)
  print("flow left-down: ", xld)

  xrd = flow_right(tiles, xr, y0, dim)
  print("flow right-down: ", xrd)

  clay_print(tiles, dim)

  print("flow> _flowcnt:", _flowcnt)
  return _flowcnt

def overflow(times, xsrc, dim=14):
  """
  Once all the left-right-downward flow has been established, evaluate the overflow tiles
  that hold steady-state H2O and reset their respective '|' values to '~'.
  For eash row of tiles, find areas bounded by clay left-right-below, anf if tile(s) are
  marked as '|', set to '~'. Perhapes simplest to navigate is bottom-up, assumimg top-down
  flow has been handled, and '|' tiles have ben set. 
  """
  global _flowcnt
  global _wetcnt

  blocked = [False for idx in range(0, dim-1)]
  for y in range(dim-2, 0, -1):
    for x in range(0, dim-1): 
      if( tiles[y][x] == '|' ):
        if( tiles[y+1][x] == '#' or tiles[y+1][x] == '~' ): 
          tiles[y][x] = '~'
          _flowcnt += -1
          _wetcnt += 1

  print("overflow> _flowcnt:", _flowcnt)
  clay_print(tiles, dim)
  return [_flowcnt, _wetcnt]

if __name__  == '__main__':
  tiles = [['.', '.', '+', '.', '.'], ['.', '#', '#', '.', '.'], ['.', '.', '.', '.', '.']]
  dimsrc = clay_init(tiles)
  dim = dimsrc[0] ; xsrc = dimsrc[1]
  cnt = flow(tiles, xsrc, 0, dim)
  cnts = overflow(tiles, xsrc, dim)


