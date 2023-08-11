package mmc.modal.visitors;

import mmc.modal.formulas.*;

import java.text.Normalizer;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

public class FormulaWriter implements FormulaVisitor  {
    private StringBuilder sb;
    private String[] actions;
    private char variable = 65;

    public FormulaWriter(String[] actions)
    {
        this.actions = actions;
        this.sb = new StringBuilder();
    }

    public String getStringValue()
    {
        return this.sb.toString();
    }
    @Override
    public void visit(BoxFormula formula) {
        String af = formula.getAction().getText();
        if(af.charAt(af.length() - 1) == '*'){
            sb.append("nu ");
            sb.append(variable);
            sb.append(".(");
            sb.append('[');
            sb.append(af.substring(0, af.length() - 1));
            sb.append(']');
            sb.append(variable);
            sb.append(" && ");
            variable++;
            formula.getFormula().accept(this);
            sb.append(')');
        } else {
            boolean negate = false;
            String[] ma;
            List<String> ma2 = new ArrayList<>();
            af = af.replaceAll("\\(","");
            af = af.replaceAll("\\)","");
            if(af.charAt(0) == '!')
            {
                negate = true;
                af = af.substring(1, af.length());
            }
            if(af.compareTo("true") == 0){
                ma = this.actions;
            } else {
                ma = af.split("\\|\\|");
            }
            for(String a : ma){
                if(!Arrays.asList(actions).contains(a))
                    System.err.println("Unknown action: " + a);
            }
            if(negate){
                for(String a : actions)
                {
                    if(!Arrays.asList(ma).contains(a)){
                        ma2.add(a);
                    }
                }
            } else {
                ma2 = Arrays.asList(ma);
            }
            sb.append('[');
            sb.append(String.join("||", ma2));
            sb.append(']');
            formula.getFormula().accept(this);
        }
    }

    @Override
    public void visit(DiamondFormula formula) {
        String af = formula.getAction().getText();
        if(af.charAt(af.length() - 1) == '*'){
            sb.append("mu ");
            sb.append(variable);
            sb.append(".(");
            sb.append('<');
            sb.append(af.substring(0, af.length() - 1));
            sb.append('>');
            sb.append(variable);
            sb.append(" || ");
            variable++;
            formula.getFormula().accept(this);
            sb.append(')');
        } else {
            boolean negate = false;
            String[] ma;
            List<String> ma2 = new ArrayList<>();
            af.replaceAll("\\(","");
            af.replaceAll("\\)","");
            if(af.charAt(0) == '!')
            {
                negate = true;
                af = af.substring(1, af.length());
            }
            if(af.compareTo("true") == 0){
                ma = this.actions;
            } else {
                ma = af.split("\\|\\|");
            }
            for(String a : ma){
                if(!Arrays.asList(actions).contains(a))
                    System.err.println("Unknown action: " + a);
            }
            if(negate){
                for(String a : actions)
                {
                    if(!Arrays.asList(ma).contains(a)){
                        ma2.add(a);
                    }
                }
            } else {
                ma2 = Arrays.asList(ma);
            }
            sb.append('<');
            sb.append(String.join("||", ma2));
            sb.append('>');
            formula.getFormula().accept(this);
        }
    }

    @Override
    public void visit(LiteralFalse formula) {
        sb.append("false");
    }

    @Override
    public void visit(LiteralTrue formula) {
        sb.append("true");
    }

    @Override
    public void visit(LogicAndFormula formula) {
        sb.append('(');
        formula.getLeft().accept(this);
        sb.append(" && ");
        formula.getRight().accept(this);
        sb.append(')');
    }

    @Override
    public void visit(LogicOrFormula formula) {
        sb.append('(');
        formula.getLeft().accept(this);
        sb.append(" || ");
        formula.getRight().accept(this);
        sb.append(')');
    }

    @Override
    public void visit(MuFormula formula) {
        sb.append("mu ");
        sb.append(formula.getRecursionVariable().getN());
        sb.append('.');
        formula.getFormula().accept(this);
    }

    @Override
    public void visit(NuFormula formula) {
        sb.append("nu ");
        sb.append(formula.getRecursionVariable().getN());
        sb.append('.');
        formula.getFormula().accept(this);
    }

    @Override
    public void visit(RecursionVariable formula) {
        sb.append(formula.getN());
    }
}
