package FTSMMC.modal.formulas;

import FTSMMC.modal.visitors.FormulaVisitor;

public interface Formula {
    int getNestingDepth();

    int getAlternationDepth();

    int getDependentAlternationDepth();

    void accept(FormulaVisitor visitor);
}
