package FTSMMC.modal.formulas;

import FTSMMC.modal.visitors.FormulaVisitor;
import FTSMMC.models.Label;

import java.util.Objects;

public class DiamondFormula extends ModalityFormula {
    public DiamondFormula(Label action, Formula formula) {
        super(action, formula);
    }

    @Override
    public void accept(FormulaVisitor visitor) {
        visitor.visit(this);
    }

    public int hashCode() {
        return Objects.hash("<>", super.hashCode());
    }

    @Override
    public String toString() {
        return String.format("<%s>%s", this.getAction(), this.getFormula());
    }
}
