package mmc.modal.formulas;

import mmc.modal.visitors.FormulaVisitor;
import mmc.models.Label;

public class DiamondFormula extends ModalityFormula implements Formula {
    public DiamondFormula(Label action, Formula formula) {
        super(action, formula);
    }

    @Override
    public void accept(FormulaVisitor visitor) {
        visitor.visit(this);
    }

    @Override
    public String toString() {
        return String.format("<%s>%s", this.getAction(), this.getFormula());
    }
}
