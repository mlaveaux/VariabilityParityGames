package mmc.models;

import mmc.models.Label;
import mmc.models.State;

public class IllegalTransitionException extends RuntimeException {
    private final State state;
    private final Label label;

    public IllegalTransitionException(State state, Label label) {
        super(String.format("State %s has no outgoing action with label %s", state, label));
        this.state = state;
        this.label = label;
    }

    public State getState() {
        return this.state;
    }

    public Label getLabel() {
        return this.label;
    }
}
