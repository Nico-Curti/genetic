#!/usr/bin/env python

import argparse
from nes_py.wrappers import BinarySpaceToDiscreteSpaceEnv
import gym_super_mario_bros
from gym_super_mario_bros.actions import COMPLEX_MOVEMENT

def parse_args():
  description = "Super Mario test rendering"

  parser = argparse.ArgumentParser(description = description)
  parser.add_argument('--movements',
                      dest='movements',
                      required=True,
                      type=int,
                      action='store',
                      nargs='+',
                      choices=range(len(COMPLEX_MOVEMENT)),
                      help='List of movements')

  args = parser.parse_args()

  if len(args.movements) <= 0:
    raise ValueError('Genome size must be non null and positive')
  return args


if __name__ == '__main__':

  args = parse_args()

  env = gym_super_mario_bros.make('SuperMarioBros-v0')
  env = BinarySpaceToDiscreteSpaceEnv(env, COMPLEX_MOVEMENT)

  done = True
  for i, step in enumerate(args.movements):
    if done:
      state = env.reset()
    _, reward, done, _ = env.step(step)
    env.render()
  env.close()
