package mmc.modal.formulas;

import mmc.modal.visitors.FormulaVisitor;

public class LogicOrFormula extends LogicFormula implements Formula {
    public LogicOrFormula(Formula left, Formula right) {
        super(left, right);
    }

    @Override
    public void accept(FormulaVisitor visitor) {
        visitor.visit(this);
    }

    @Override
    public String toString() {
        return String.format("(%s || %s)", this.getLeft(), this.getRight());
    }
}
