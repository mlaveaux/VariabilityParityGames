package mmc.modal.formulas;

import java.util.Objects;

public abstract class LogicFormula implements Formula {
    private final Formula left;
    private final Formula right;

    public LogicFormula(Formula left, Formula right) {
        this.left = left;
        this.right = right;
    }

    public Formula getLeft() {
        return this.left;
    }

    public Formula getRight() {
        return this.right;
    }

    @Override
    public int getNestingDepth() {
        return Math.max(this.left.getNestingDepth(), this.right.getNestingDepth());
    }

    @Override
    public int getAlternationDepth() {
        return Math.max(this.left.getAlternationDepth(), this.right.getAlternationDepth());
    }

    @Override
    public int getDependentAlternationDepth() {
        return Math.max(this.left.getDependentAlternationDepth(), this.right.getAlternationDepth());
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;
        LogicFormula that = (LogicFormula) o;
        return Objects.equals(this.left, that.left) &&
                Objects.equals(this.right, that.right);
    }

    @Override
    public int hashCode() {
        return Objects.hash(this.left, this.right);
    }

    public abstract String toString();
}
