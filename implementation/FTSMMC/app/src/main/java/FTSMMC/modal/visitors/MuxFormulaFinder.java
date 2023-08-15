package FTSMMC.modal.visitors;

import FTSMMC.modal.formulas.FixedPointFormula;
import FTSMMC.modal.formulas.MuFormula;
import FTSMMC.modal.formulas.RecursionVariable;

import java.util.ArrayList;
import java.util.List;

public class MuxFormulaFinder extends RecursiveVisitor implements FormulaVisitor {
    private final RecursionVariable recursionVariable;
    private final List<MuFormula> muFormulas;

    private MuxFormulaFinder(RecursionVariable recursionVariable) {
        this.recursionVariable = recursionVariable;
        this.muFormulas = new ArrayList<>();
    }

    public static List<MuFormula> findMuxFormulas(FixedPointFormula formula) {
        MuxFormulaFinder muxFormulaFinder = new MuxFormulaFinder(formula.getRecursionVariable());
        formula.accept(muxFormulaFinder);

        return muxFormulaFinder.muFormulas;
    }

    @Override
    public void visit(MuFormula formula) {
        if (formula.getVariableMatcher().getFree().contains(this.recursionVariable)) {
            this.muFormulas.add(formula);
        }
        super.visit(formula);
    }
}
