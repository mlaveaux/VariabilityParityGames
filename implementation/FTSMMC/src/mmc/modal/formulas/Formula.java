package mmc.modal.formulas;

import mmc.modal.visitors.FormulaVisitor;

public interface Formula {
    int getNestingDepth();
    int getAlternationDepth();
    int getDependentAlternationDepth();
    void accept(FormulaVisitor visitor);
}
