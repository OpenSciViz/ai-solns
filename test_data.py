#!/usr/bin/env python

from random import randrange as rand
from copy import deepcopy
# import numpy as np

def clay_init(tiles, dim=14):
  """
  Initialize a square claypit of tiles with dimensions width == height = dim.
  Return the x coord index of the H2O source tile at y == 0. 
  """
  #clay0 = ['.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.','.', '.']
  #clay[y][x] =  '.' or '#' or '|' or  '~' ... i.e. 'sand' or 'clay' or 'h2o flow' or 'h2o saturation'

  clay0 = []
  for idx in range(0,dim):
    clay0.append('.')
# print(clay0)

  clay = []
  for idx in range(0,dim):
    clay.append(deepcopy(clay0)) # np.zeros((14,14))

  xsrc = rand(dim/4, dim-3) # place the fountain source randomly, but within a few tiles of the borders
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

def clay_readfile(clay_tiles, filename='./testclay.txt', dim=14):
   bcnt = len(clay_tiles)
   return bcnt

def flow_down(tiles, yy0=1, x0=7, dim=14):
    if(tiles[y][x0-1-x] == '#'):
      return x0-1-x
    # if we get here no clay tile found below x, y0
    return -1
  

if __name__ == '__main__':
  tiles = [['.', '.', '+', '.', '.'], ['.', '#', '#', '.', '.'], ['.', '.', '.', '.', '.']]
  x = clay_init(tiles)


