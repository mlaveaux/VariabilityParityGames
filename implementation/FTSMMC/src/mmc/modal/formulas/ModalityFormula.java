package mmc.modal.formulas;

import mmc.models.Label;

import java.util.Objects;

public abstract class ModalityFormula implements Formula {
    private final Label action;
    private final Formula formula;

    protected ModalityFormula(Label action, Formula formula) {
        Objects.requireNonNull(action);
        this.action = action;
        Objects.requireNonNull(formula);
        this.formula = formula;
    }

    public Label getAction() {
        return this.action;
    }

    public Formula getFormula() {
        return this.formula;
    }

    @Override
    public int getNestingDepth() {
        return this.formula.getNestingDepth();
    }

    @Override
    public int getAlternationDepth() {
        return this.formula.getAlternationDepth();
    }

    @Override
    public int getDependentAlternationDepth() {
        return this.formula.getDependentAlternationDepth();
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;
        ModalityFormula that = (ModalityFormula) o;
        return Objects.equals(this.action, that.action) &&
            Objects.equals(this.formula, that.formula);
    }

    @Override
    public int hashCode() {
        return Objects.hash(this.action, this.formula);
    }

    public abstract String toString();
}
