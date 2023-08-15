package FTSMMC.modal.formulas;

import FTSMMC.modal.visitors.FormulaVisitor;
import FTSMMC.modal.visitors.NuFormulaFinder;
import FTSMMC.modal.visitors.NuxFormulaFinder;

public class MuFormula extends FixedPointFormula {
    public MuFormula(RecursionVariable recursionVariable, Formula formula) {
        super(recursionVariable, formula);
    }

    @Override
    public int getAlternationDepth() {
        return 1 + NuFormulaFinder.findNuFormulas(this).stream()
                .mapToInt(Formula::getAlternationDepth)
                .reduce(Integer::max)
                .orElse(0);
    }

    @Override
    public int getDependentAlternationDepth() {
        return Math.max(
                this.getFormula().getDependentAlternationDepth(),
                1 + NuxFormulaFinder.findNuxFormulas(this).stream()
                        .mapToInt(Formula::getDependentAlternationDepth)
                        .reduce(Integer::max)
                        .orElse(0));
    }

    @Override
    public void accept(FormulaVisitor visitor) {
        visitor.visit(this);
    }

    @Override
    public String toString() {
        return String.format("mu %s.%s", this.getRecursionVariable(), this.getFormula());
    }
}
