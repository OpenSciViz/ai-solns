#!/usr/bin/env python

from random import randrange as rand
from copy import deepcopy
import numpy as np

def clay_init(tiles, dim=14):
  """
  Initialize a square claypit of tiles with dimensions width == height = dim.
  Return the x coord index of the H2O source tile at y == 0. 
  """
  #clay0 = ['.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.','.', '.']

  clay0 = []
  for idx in range(0,dim):
    clay0.append('.')
# print(clay0)

  clay = []
  for idx in range(0,dim):
    clay.append(deepcopy(clay0)) # np.zeros((14,14))

  xsrc = rand(1, 13)
  y = 0 ; clay[y][xsrc] = '+'
  print('{0: >2}'.format(y), clay[y])

  for y in range(1,14):
    for x in range(0,14):
      yesno = rand(0,3)
      if( yesno == 0 ):
        clay[y][x] =  '#' # '|' '~'
    print('{0: >2}'.format(y), clay[y])

  tiles = deepcopy(clay)
  return xsrc

def flow_down(tiles, y0=1, x0=7, dim=14):
  """
  Return index of first clay tile found below current/origin tile and if none are found, return -1.
  Downward flow is possible (not blocked by clay), if -1 is returned, or an index > (y0 + 1) is
  returned, otherwise downward flow is blocked.
  """
  for y in range(1+y0, dim):
    if(tiles[y][x0] == '#'):
      return y
    # if we get here no clay tile found below x, y0
    return -1
  
def flow_right(tiles, y0=1, x0=7, dim=14):
  """
  Return index of first clay tile found right of current/origin tile and if none are found, return -1.
  Rightward flow is possible (not blocked by clay), if -1 is returned, or an index > x+1 is returned,
  otherwise rightwardward flow is blocked.
  """
  for x in range(1+x0, dim):
    if(tiles[y0][x] == '#'):
      return x
    # if we get here no clay tile found right of x0, y0
    return -1
  
def flow_left(tiles, y0=1, x0=7, dim=14):
  """
  Return index of first clay tile found left of current/origin tile and if none are found, return -1.
  Leftward flow is possible (not blocked by clay), if -1 is returned, or an index < x-1 is returned,
  otherwise leftwardward flow is blocked.
  """
  for x in range(0, x0):
    if(tiles[y][x0-1-x] == '#'):
      return x0-1-x
    # if we get here no clay tile found below x, y0
    return -1
  

if __name__ == '__main__':
  tiles = [['.', '.', '+', '.', '.'], ['.', '#', '#', '.', '.'], ['.', '.', '.', '.', '.']]
  x = clay_init(tiles)


