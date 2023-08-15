package FTSMMC.modal;

import FTSMMC.modal.formulas.*;
import FTSMMC.modal.visitors.FixedPointVisitor;
import FTSMMC.modal.visitors.ScopeRecursionVariables;
import FTSMMC.models.Label;

import java.lang.reflect.InvocationTargetException;
import java.util.Arrays;
import java.util.HashSet;
import java.util.Set;


public class ModalParser {
    private static final Set<Character> true_literal_first_set = new HashSet<>(Arrays.asList('t'));
    private static final Set<Character> false_literal_first_set = new HashSet<>(Arrays.asList('f'));
    private static final Set<Character> recursion_variable_first_set = new HashSet<>(Arrays.asList('A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z'));
    private static final Set<Character> logic_formula_first_set = new HashSet<>(Arrays.asList('('));
    private static final Set<Character> mu_formula_first_set = new HashSet<>(Arrays.asList('m'));
    private static final Set<Character> nu_formula_first_set = new HashSet<>(Arrays.asList('n'));
    private static final Set<Character> diamond_formula_first_set = new HashSet<>(Arrays.asList('<'));
    private static final Set<Character> box_formula_first_set = new HashSet<>(Arrays.asList('['));
    private static final Set<Character> and_operator_first_set = new HashSet<>(Arrays.asList('&'));
    private static final Set<Character> or_operator_first_set = new HashSet<>(Arrays.asList('|'));
    private static final Set<Character> operator_first_set = new HashSet<>(Arrays.asList('|','&'));
    private static final Set<Character> action_name_first_set = new HashSet<>(Arrays.asList('1','2','3','4', '5', '6', '7','8','9','0','(',')','!','*','|','A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z','a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z','_'));

    private Set<Character> formula_first_set;
    private String formula;
    private int i;

    public ModalParser(String formula)
    {
        this.formula = formula;

        this.formula_first_set = new HashSet<>();
        this.formula_first_set.addAll(true_literal_first_set);
        this.formula_first_set.addAll(false_literal_first_set);
        this.formula_first_set.addAll(recursion_variable_first_set);
        this.formula_first_set.addAll(logic_formula_first_set);
        this.formula_first_set.addAll(mu_formula_first_set);
        this.formula_first_set.addAll(nu_formula_first_set);
        this.formula_first_set.addAll(diamond_formula_first_set);
        this.formula_first_set.addAll(box_formula_first_set);
    }

    public Formula parse() throws ParseException {
        i = 0;
        skipWhiteSpace();
        char c = getChar();
        if(formula_first_set.contains(c))
        {
            Formula f = parseFormula();
            skipWhiteSpace();
            if(i != formula.length())
            {
                throw new ParseException("End of formula expected.", i);
            }
            callAllVisitors(f);
            return f;
        }
        else
        {
            throw new ParseException("Unknown start of the formula.", i);
        }
    }

    private void callAllVisitors(Formula f) {
        scopeRecursionVariables(f);
        fixedPointsVisitor(f);
    }

    private void scopeRecursionVariables(Formula f){
        ScopeRecursionVariables srv = new ScopeRecursionVariables();
        f.accept(srv);
    }

    private void fixedPointsVisitor(Formula f) {
        FixedPointVisitor fpv = new FixedPointVisitor();
        f.accept(fpv);
    }

    private Formula parseFormula() throws ParseException {
        char c = getChar();
        if(true_literal_first_set.contains(c)) {
            return parseTrueLiteral();
        } else if(false_literal_first_set.contains(c)) {
            return parseFalseLiteral();
        } else if(recursion_variable_first_set.contains(c)) {
            return parseRecursionVariable();
        } else if(logic_formula_first_set.contains(c)) {
            return parseLogicFormula();
        } else if(mu_formula_first_set.contains(c)) {
            return parseMuFormula();
        } else if(nu_formula_first_set.contains(c)) {
            return parseNuFormula();
        } else if(diamond_formula_first_set.contains(c)) {
            return parseDiamondFormula();
        } else if(box_formula_first_set.contains(c)) {
            return parseBoxFormula();
        }
        assert false;
        return null;
    }

    private Formula parseMuFormula() throws ParseException {
        expect("mu");
        requireWhiteSpace();
        RecursionVariable r;
        Formula f;
        if(recursion_variable_first_set.contains(getChar()))
        {
            r = parseRecursionVariable();
        } else {
            throw new ParseException("Expected recursion variable.",i);
        }
        expect(".");
        skipWhiteSpace();
        if(formula_first_set.contains(getChar()))
        {
            f = parseFormula();
        } else {
            throw new ParseException("Expected formula.",i);
        }
        return new MuFormula(r, f);
    }

    private Formula parseNuFormula() throws ParseException {
        expect("nu");
        requireWhiteSpace();
        RecursionVariable r;
        Formula f;
        if(recursion_variable_first_set.contains(getChar()))
        {
            r = parseRecursionVariable();
        } else {
            throw new ParseException("Expected recursion variable.",i);
        }
        expect(".");
        skipWhiteSpace();
        if(formula_first_set.contains(getChar()))
        {
            f = parseFormula();
        } else {
            throw new ParseException("Expected formula.",i);
        }
        return new NuFormula(r, f);
    }

    private Formula parseDiamondFormula() throws ParseException {
        expect("<");
        Formula f;
        String action;
        skipWhiteSpace();
        if(action_name_first_set.contains(getChar())){
            action = parseActionName();
        } else {
            throw new ParseException("Action expected.",i);
        }
        expect(">");
        skipWhiteSpace();
        if(formula_first_set.contains(getChar())){
            f = parseFormula();
        } else {
            throw new ParseException("formula expected.",i);
        }
        return new DiamondFormula(new Label(action, false), f);
    }

    private Formula parseBoxFormula() throws ParseException {
        expect("[");
        Formula f;
        String action;
        skipWhiteSpace();
        if(action_name_first_set.contains(getChar())){
            action = parseActionName();
        } else {
            throw new ParseException("Action expected.",i);
        }
        expect("]");
        skipWhiteSpace();
        if(formula_first_set.contains(getChar())){
            f = parseFormula();
        } else {
            throw new ParseException("formula expected.",i);
        }
        return new BoxFormula(new Label(action, false), f);
    }

    private String parseActionName() throws ParseException {
        StringBuilder n = new StringBuilder();
        char c;
        while(action_name_first_set.contains((c = getChar())))
        {
            n.append(c);
            i++;
        }
        return n.toString();
    }

    private Formula parseTrueLiteral() throws ParseException {
        expect("true");
        skipWhiteSpace();
        return new LiteralTrue();
    }
    private Formula parseFalseLiteral() throws ParseException {
        expect("false");
        skipWhiteSpace();
        return new LiteralFalse();
    }

    private RecursionVariable parseRecursionVariable() throws ParseException {
        char n = getChar();
        i++;
        skipWhiteSpace();
        return new RecursionVariable(n);
    }

    private Formula parseLogicFormula() throws ParseException {
        expect("(");
        Formula f;
        Formula g;
        Class<? extends LogicFormula> c;
        skipWhiteSpace();
        if(formula_first_set.contains(getChar())) {
            f = parseFormula();
        } else {
            throw new ParseException("formula expected.",i);
        }
        if(operator_first_set.contains(getChar())) {
            c = parseOperator();
        } else {
            throw new ParseException("Operator expected.", i);
        }
        if(formula_first_set.contains(getChar())) {
            g = parseFormula();
        } else {
            throw new ParseException("formula expected.",i);
        }
        expect(")");
        skipWhiteSpace();
        try {
            return c.getConstructor(Formula.class, Formula.class).newInstance(f, g);
        } catch (InstantiationException |
                IllegalAccessException |
                NoSuchMethodException |
                InvocationTargetException e) {
            throw new RuntimeException(String.format("Cannot instantiate correct Logic Formula at %s", i), e);
        }
    }

    private Class<? extends LogicFormula> parseOperator() throws ParseException
    {
        if(and_operator_first_set.contains(getChar()))
        {
            return parseLogicAndOperator();
        } else if(or_operator_first_set.contains(getChar()))
        {
            return parseLogicOrOperator();
        }
        assert false;
        return null;
    }

    private Class<LogicAndFormula> parseLogicAndOperator() throws ParseException
    {
        expect("&&");
        skipWhiteSpace();
        return LogicAndFormula.class;
    }

    private Class<LogicOrFormula> parseLogicOrOperator() throws ParseException
    {
        expect("||");
        skipWhiteSpace();
        return LogicOrFormula.class;
    }

    private void expect(String e) throws ParseException
    {
        int l = e.length();
        if(i > formula.length() - l)
        {
            throw new ParseException("Unexpected end of formula.",i);
        }
        if(formula.substring(i, i + l).compareTo(e) != 0)
        {
            throw new ParseException("Expected: " + e,i);
        } else {
            i += l;
        }
    }

    /**
     * Skip whitespace and comments
     */
    private void skipWhiteSpace() throws ParseException {
        if(i == formula.length())
            return;
        char c = getChar();
        while(c == ' ' || c == '\r' || c == '\n' || c == '\t') {
            i++;
            if(i == formula.length())
                return;
            c = formula.charAt(i);
        }
        if(c == '%')
        {
            do {
                i++;
                if(i == formula.length())
                    return;
                c = getChar();
            }
            while(c != '\n');
            this.skipWhiteSpace();
        }
    }

    private void requireWhiteSpace() throws ParseException {
        char c = getChar();
        if(c != ' ' && c != '\r' && c != '\n' && c != '\t' && c != '%')
        {
            throw new ParseException("Whitespace required.",i);
        }
        this.skipWhiteSpace();
    }

    private char getChar () throws ParseException {
        if(i == formula.length())
        {
            throw new ParseException("Unexpected end of formula.",i);
        }
        return formula.charAt(i);
    }
}
