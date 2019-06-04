package mmc.modal.formulas;

import mmc.modal.visitors.FormulaVisitor;
import mmc.modal.visitors.NuFormulaFinder;
import mmc.modal.visitors.NuxFormulaFinder;

public class MuFormula extends FixedPointFormula implements Formula {
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
                .orElse(0)
        );
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
