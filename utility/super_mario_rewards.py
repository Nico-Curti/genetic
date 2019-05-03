#!/usr/bin/env python

import argparse
from nes_py.wrappers import BinarySpaceToDiscreteSpaceEnv
import gym_super_mario_bros
from gym_super_mario_bros.actions import RIGHT_ONLY, SIMPLE_MOVEMENT, COMPLEX_MOVEMENT

MOVEMENTS = COMPLEX_MOVEMENT

def parse_args():
  description = "Super Mario rewards calculator"

  parser = argparse.ArgumentParser(description = description)
  parser.add_argument('--movements',
                      dest='movements',
                      required=True,
                      type=int,
                      action='store',
                      nargs='+',
                      choices=range(len(MOVEMENTS)),
                      help='List of movements')
  parser.add_argument('--dimension',
                      dest='dim',
                      required=False,
                      type=int,
                      action='store',
                      help='Genome size (aka number of movements)',
                      default=5)

  args = parser.parse_args()

  if args.dim <= 0:
    raise ValueError('Genome size must be non null and positive')
  if args.dim != len(args.movements):
    raise ValueError('Invalid number of movements! It must be the same of dim parameter')
  return args


if __name__ == '__main__':

  args = parse_args()

  env = gym_super_mario_bros.make('SuperMarioBros-v0')
  env = BinarySpaceToDiscreteSpaceEnv(env, MOVEMENTS)

  done = True
  old = 40

  for i, step in enumerate(args.movements):
    if done:
      state = env.reset()
    s, reward, done, info = env.step(step)

    state = s
    if i % 50 == 0:
      if old == info['x_pos']: break
      else: old = info['x_pos']

  fitness =  - reward / 30 + (args.dim - info['x_pos']) / args.dim + abs(3 - info['life']) / 3 - info['score'] / ((info['score']+1e-10) * 100)
  env.close()

  print(fitness)
