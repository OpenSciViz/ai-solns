#!/usr/bin/env python

# import numpy as np

_flowcnt = 0

def flow_right(tiles, x0=0, y0=0, dim=14):
  """
  Return index of first clay tile found right of current/origin tile and if none are found, return -1.
  Rightward flow is possible (not blocked by clay), if -1 is returned, or an index > x+1 is returned,
  otherwise rightwardward flow is blocked.Set wet tiles to either '|' or '+' along path to
  clay or if no clay all should be set to '|'. If flow is blocked up then upper '|' may get 
  reset to '~' once steady-stete is achieved. 
  """
  global _flowcnt
  for xr in range(1+x0, dim):
    if(tiles[y0][xr] == '#'):
      return xr
    else:
      tiles[y]xr] = '|' ; _flowcnt += 1 # keep flowing right
    # if we get here no clay tile found right of x0, y0
    return -1
  
def flow_left(tiles, x0=0, y0=0, dim=14):
  """
  Return index of first clay tile found left of current/origin tile and if none are found, return -1.
  Leftward flow is possible (not blocked by clay), if -1 is returned, or an index < x-1 is returned,
  otherwise leftwardward flow is blocked. Set wet tiles to either '|' or '+' along path to
  clay or if no clay all should be set to '|'. If flow is blocked up then upper '|' may get 
  reset to '~' once steady-stete is achieved. 
  """
  global _flowcnt
  for x in range(0, x0):
    xl = x0-x-1
    if(tiles[y][xl] == '#'):
      return x0-1-x
    else:
      tiles[y][xl] = '|' ; _flowcnt += 1 # keep flowing left
    # if we get here no clay tile found left of x0, y0
    return -1

def flow_down(tiles, x0=0, y0=0, dim=14):
  """
  Return index of first clay tile found below current/origin tile and if none are found, return -1.
  Downward flow is possible (not blocked by clay), if -1 is returned, or an index > (y0 + 1) is
  returned, otherwise downward flow is blocked. Set wet tiles to either '|' or '+' along path to
  clay or if no clay all should be set to '|'. If flow is blocked up then upper '|' may get 
  reset to '~' once steady-stete is achieved. 
  """
  global _flowcnt
  yclay = -1
  for y in range(1+y0, dim):
    if(tiles[y][x0] == '#'):
      yclay = y
  
  if( yclay < 0 ):
    # set all tiles below to '|' and return count of '|'
    for y in range(y0, dim-y0):
      tiles[y][x0] = '|' ; _flowcnt += 1
    return dim-y0

  # set the deepest non-clay tile to '~' and those above to '|'
  tiles[yclay-1][x0] = '~'
  for y in range(y0, down_clay-y0-1):
    tiles[y][x0] = '|' ; _flowcnt += 1

  return yclay
  
def flow(tiles, x0=0, y0=0, dim=14):
  """
  Inspect tiles to left and right of current (x0, y0) and alse directly below
  to left and right of (x0, y0+1). If symmetric, flow should go both ways and
  continue down of fill up. Otherwise, flow proceeds down nearest open (non-clay)
  path, etc. 
  """
  global _flowcnt
  y = down_clay = flow_down(tiles, x0, y0, dim)
  xl = left_clay = flow_left(tiles, x0, y, dim)
  xr = right_clay = flow_right(tiles, x0, y, dim)

  while( xl >= 0 && y < dim ):
    y = down_clay = flow_down(tiles, x0, y0, dim)
    xl = flow_left(tiles, xl, y, dim)

  while( xr >= 0 && xr < dim && y < dim ):
    y = down_clay = flow_down(tiles, x0, y0, dim)
    xr = flow_left(tiles, xr, y, dim)

  return _flowcnt

def overflow(times, x0-0, y0=0, dim=14):
  """
  Once all the left-right-downward flow has been established, evaluate the overflow tiles
  that hold steady-state H2O and reset their respective '|' values to '~'.
  For eash row od tiles, find areas bounnded by clay left-right-below, anf if tile(s) are
  marked as '|', set to '~'.
  """

if __name__  == '__main__':
  tiles = [['.', '.', '+', '.', '.'], ['.', '#', '#', '.', '.'], ['.', '.', '.', '.', '.']]
  x = clay_init(tiles)


