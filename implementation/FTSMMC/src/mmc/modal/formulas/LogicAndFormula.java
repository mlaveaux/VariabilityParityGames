package mmc.modal.formulas;

import mmc.modal.visitors.FormulaVisitor;

public class LogicAndFormula extends LogicFormula implements Formula {
    public LogicAndFormula(Formula left, Formula right) {
        super(left, right);
    }

    @Override
    public void accept(FormulaVisitor visitor) {
        visitor.visit(this);
    }

    @Override
    public String toString() {
        return String.format("(%s && %s)", this.getLeft(), this.getRight());
    }
}
