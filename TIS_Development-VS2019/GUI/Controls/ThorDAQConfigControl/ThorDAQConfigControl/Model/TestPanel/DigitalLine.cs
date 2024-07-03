using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ThorDAQConfigControl.Model
{
    public enum Direction
    {
        Output = 0,
        Input
    }

    public enum State
    {
        Low = 0,
        High
    }

    public class DigitalLine
    {
        public int StartPos;
        public int EndPos;
        private int Count;
        public Direction[] Directions;
        public State[] States;

        public DigitalLine(int start, int end)
        {
            StartPos = start;
            EndPos = end;
            Count = end - start;
            Directions = new Direction[Count];
            States = new State[Count];
        }

        public void SetAllDirection(Direction dir)
        {
            for (int i = 0; i < Count; i++)
                Directions[i] = dir;
        }

        public void SetDirection(int pos, Direction dir)
        {
            Directions[pos] = dir;
        }

        public Direction GetDirection(int pos)
        {
            return Directions[pos];
        }

        public void SetAllState(State st)
        {
            for (int i = 0; i < Count; i++)
                States[i] = st;
        }

        public void SetState(int pos, State st)
        {
            States[pos] = st;
        }

        public State GetState(int pos)
        {
            return States[pos];
        }
    }
}
