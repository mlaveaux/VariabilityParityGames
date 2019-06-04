package mmc.models;

import java.util.*;

public class State {
    private final int number;
    private final Map<Label, Set<State>> transitions;
    // Used to prevent recursion in hashCode
    private final Map<Label, Set<Integer>> simpleTransitions;

    public State(int number) {
        this.number = number;
        this.transitions = new HashMap<>();
        this.simpleTransitions = new HashMap<>();
    }

    public int getNumber() {
        return this.number;
    }

    public Set<Label> getTransitionLabels() {
        return this.transitions.keySet();
    }

    public boolean addTransition(Label label, State next) {
        Set<State> possibleStates = this.transitions.computeIfAbsent(label, k -> new HashSet<>());
        Set<Integer> possibleStateNumbers = this.simpleTransitions.computeIfAbsent(label, k -> new HashSet<>());

        boolean stateAdded = possibleStates.add(next);
        boolean numberAddded = possibleStateNumbers.add(next.getNumber());
        if(stateAdded != numberAddded) {
            throw new IllegalStateException("Added transition while object was in an illegal state");
        }

        return stateAdded;
    }

    public Set<State> transition(Label label) {
        Set<State> possibleStates = this.transitions.get(label);
        if (possibleStates == null) {
            throw new IllegalTransitionException(this, label);
        }
        return possibleStates;
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;
        State state = (State) o;
        return this.number == state.number &&
            Objects.equals(this.transitions, state.transitions) &&
            Objects.equals(this.simpleTransitions, state.simpleTransitions);
    }

    @Override
    public int hashCode() {
        return Objects.hash(this.number, this.simpleTransitions);
    }

    @Override
    public String toString() {
        return "State{" +
            "number=" + this.number +
            //", transitions=" + this.simpleTransitions +
            '}';
    }
}
