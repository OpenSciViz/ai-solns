#!/usr/bin/env python

from random import randrange as rand
from copy import deepcopy
import numpy as np

def clay_init(dim=14):
  """
  Initialize a square claypit od dimension widh == height = dim. 
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


if __name__ == '__main__':
  clay_init()

