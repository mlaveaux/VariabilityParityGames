package mmc.modal.formulas;

import mmc.modal.visitors.FormulaVisitor;

import java.util.Objects;

public class RecursionVariable implements Formula {
    private final char n;
    private int identifier;

    public RecursionVariable(char n)
    {
        this.n = n;
        this.identifier = 0;
    }

    public void setIdentifier(int identifier){
        this.identifier = identifier;
    }

    public int getIdentifier(){
        return this.identifier;
    }

    public char getN(){
        return n;
    }

    @Override
    public int getNestingDepth() {
        return 0;
    }

    @Override
    public int getAlternationDepth() {
        return 0;
    }

    @Override
    public int getDependentAlternationDepth() {
        return 0;
    }

    @Override
    public void accept(FormulaVisitor visitor) {
        visitor.visit(this);
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;
        RecursionVariable that = (RecursionVariable) o;
        return this.identifier == that.identifier;
    }

    @Override
    public int hashCode() {
        return Objects.hash(this.identifier);
    }

    @Override
    public String toString() {
        return String.format("%s{%d}", this.n, this.identifier);
    }
}
