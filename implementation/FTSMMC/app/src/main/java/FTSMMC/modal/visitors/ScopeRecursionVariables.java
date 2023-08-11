package mmc.modal.visitors;

import mmc.modal.formulas.*;

import java.security.InvalidParameterException;
import java.util.HashMap;
import java.util.Map;

/**
 * Assign a unique 
 */
public class ScopeRecursionVariables extends RecursiveVisitor
{
    private final Map<Character, Integer> recursionMap;
    private int identifier_counter = 0;

    public ScopeRecursionVariables() {
        this.recursionMap = new HashMap<>();
    }

    @Override
    public void visit(MuFormula formula) {
        char n = formula.getRecursionVariable().getN();
        Integer id = recursionMap.put(n, ++identifier_counter);
        super.visit(formula);
        recursionMap.put(n, id);
    }

    @Override
    public void visit(NuFormula formula) {
        char n = formula.getRecursionVariable().getN();
        Integer id = recursionMap.put(n, ++identifier_counter);
        super.visit(formula);
        recursionMap.put(n, id);
    }

    @Override
    public void visit(RecursionVariable formula) {
        char n = formula.getN();
        if(!recursionMap.containsKey(n)){
            throw new InvalidParameterException("Recursion variable " + n +" used but not declared (in scope).");
        }
        formula.setIdentifier(recursionMap.get(n));
        super.visit(formula);
    }
}
