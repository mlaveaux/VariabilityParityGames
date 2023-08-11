package mmc.modal.formulas;

import mmc.modal.visitors.FormulaVisitor;
import mmc.modal.visitors.MuFormulaFinder;
import mmc.modal.visitors.MuxFormulaFinder;

public class NuFormula extends FixedPointFormula implements Formula {
    public NuFormula(RecursionVariable recursionVariable, Formula formula) {
        super(recursionVariable, formula);
    }

    @Override
    public int getAlternationDepth() {
        return 1 + MuFormulaFinder.findMuFormulas(this).stream()
                .mapToInt(Formula::getAlternationDepth)
                .reduce(Integer::max)
                .orElse(0);
    }

    @Override
    public int getDependentAlternationDepth() {
        return Math.max(
            this.getFormula().getDependentAlternationDepth(),
            1 + MuxFormulaFinder.findMuxFormulas(this).stream()
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
        return String.format("nu %s.%s", this.getRecursionVariable(), this.getFormula());
    }
}
