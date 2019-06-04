package mmc.modal.formulas;

import mmc.modal.visitors.FormulaVisitor;

public class LiteralFalse implements Formula {
    @Override
    public int getNestingDepth() {
        return 0;
    }

    @Override
    public int getAlternationDepth() {
        return 0;
    }

    @Override
    public int getDependentAlternationDepth() {
        return 0;
    }

    @Override
    public void accept(FormulaVisitor visitor) {
        visitor.visit(this);
    }

    @Override
    public String toString() {
        return "false";
    }
}
