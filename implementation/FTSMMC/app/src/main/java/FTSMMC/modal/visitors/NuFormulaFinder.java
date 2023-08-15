package FTSMMC.modal.visitors;

import FTSMMC.modal.formulas.FixedPointFormula;
import FTSMMC.modal.formulas.NuFormula;

import java.util.ArrayList;
import java.util.List;

public class NuFormulaFinder extends RecursiveVisitor {
    private final List<NuFormula> nuFormulas;

    private NuFormulaFinder() {
        this.nuFormulas = new ArrayList<>();
    }

    public static List<NuFormula> findNuFormulas(FixedPointFormula formula) {
        NuFormulaFinder nuFormulaFinder = new NuFormulaFinder();
        formula.accept(nuFormulaFinder);

        return nuFormulaFinder.nuFormulas;
    }

    @Override
    public void visit(NuFormula formula) {
        this.nuFormulas.add(formula);
        super.visit(formula);
    }
}
