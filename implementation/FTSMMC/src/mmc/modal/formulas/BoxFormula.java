package mmc.modal.formulas;

import mmc.modal.visitors.FormulaVisitor;
import mmc.models.Label;

import java.util.Objects;

public class BoxFormula extends ModalityFormula implements Formula {
    public BoxFormula(Label action, Formula formula) {
        super(action, formula);
    }

    @Override
    public void accept(FormulaVisitor visitor) {
        visitor.visit(this);
    }

    public int hashCode() {
        return Objects.hash( "[]", super.hashCode());
    }
    @Override
    public String toString() {
        return String.format("[%s]%s", this.getAction(), this.getFormula());
    }
}
