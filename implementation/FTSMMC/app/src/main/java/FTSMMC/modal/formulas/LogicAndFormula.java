package FTSMMC.modal.formulas;

import FTSMMC.modal.visitors.FormulaVisitor;

import java.util.Objects;

public class LogicAndFormula extends LogicFormula {
    public LogicAndFormula(Formula left, Formula right) {
        super(left, right);
    }

    @Override
    public void accept(FormulaVisitor visitor) {
        visitor.visit(this);
    }

    public int hashCode() {
        return Objects.hash("or", super.hashCode());
    }

    @Override
    public String toString() {
        return String.format("(%s && %s)", this.getLeft(), this.getRight());
    }
}
