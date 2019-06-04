package mmc.models;

import java.util.*;

public class Lts {
    private final State[] states;
    private final int startIndex;

    public Lts() {
        this(1);
    }

    public Lts(int states) {
        this(states, 0);
    }

    public Lts(int states, int startIndex) {
        if (states < 1) {
            throw new IllegalArgumentException("An LTS requires a positive number of states");
        }
        if (startIndex < 0 || startIndex >= states) {
            throw new IndexOutOfBoundsException("startIndex must adhere to states variable");
        }
        this.states = new State[states];
        for (int i = 0; i < states; i++) {
            this.states[i] = new State(i);
        }
        this.startIndex = startIndex;
    }

    public State getStart() {
        return this.states[this.startIndex];
    }

    public State getState(int number) {
        if (number < 0 || number > this.states.length) {
            throw new IndexOutOfBoundsException(String.format("Unknown state with number %s", number));
        }
        State state = this.states[number];

        return state;
    }

    public State[] getStates(){
        return this.states;
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;
        Lts lts = (Lts) o;
        return this.startIndex == lts.startIndex &&
            Arrays.equals(this.states, lts.states);
    }

    @Override
    public int hashCode() {
        int result = Objects.hash(this.startIndex);
        result = 31 * result + Arrays.hashCode(this.states);
        return result;
    }

    @Override
    public String toString() {
        return "Lts{" +
            "states=" + Arrays.toString(this.states) +
            ", startIndex=" + this.startIndex +
            '}';
    }
}
