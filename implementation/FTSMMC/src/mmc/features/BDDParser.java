package mmc.features;

import net.sf.javabdd.BDD;

public class BDDParser {
    public static BDD parseBDD(String BDD, FeatureDiagram fe) throws Exception {
        BDDParser a  = new BDDParser(BDD,0);
        a.parse(fe);
        return a.generateFE(fe);
    }
    private BDDParser left;
    private BDDParser right;
    private String variable;
    private boolean leaf = false;
    private BDD leafvalue;

    private String BDDstr;
    private int posorg;
    private int pos;

    private BDDParser(String BDD, int pos)
    {
        this.BDDstr = BDD;
        this.posorg = pos;
        this.pos = pos;
    }

    protected int parse(FeatureDiagram fe) throws Exception {
        skipSpaces();
        if(textIs("tt"))
        {
            this.leaf = true;
            this.leafvalue = fe.factory.one();
        } else if(textIs("ff"))
        {
            this.leaf = true;
            this.leafvalue = fe.factory.zero();
        } else if(textIs("node(")) {
            this.variable = textUntil(',');
            left = new BDDParser(BDDstr, pos);
            pos += left.parse(fe);
            skipSpaces();
            if(!textIs(","))
                throw new Exception("Expected ',' at " + String.valueOf(pos));
            right = new BDDParser(BDDstr, pos);
            pos += right.parse(fe);
            if(!textIs(")"))
                throw new Exception("Expected ')' at " + String.valueOf(pos));
        } else {
            throw new Exception("Unknown characted " + BDDstr.charAt(pos) + " at " + String.valueOf(pos));
        }

        return this.pos - this.posorg;
    }

    private String textUntil(char endChar)
    {
        StringBuilder sb = new StringBuilder();
        char c;
        while((c = BDDstr.charAt(pos++)) != endChar)
            sb.append(c);
        return sb.toString();
    }

    private boolean textIs(String assertion){
        if(BDDstr.substring(pos, pos + assertion.length()).compareTo(assertion) == 0)
        {
            this.pos += assertion.length();
            return true;
        } else
            return false;
    }

    private void skipSpaces(){
        while(BDDstr.charAt(pos) == ' ')
            pos++;
    }

    private BDD generateFE(FeatureDiagram fe)
    {
        if(this.leaf)
            return this.leafvalue;
        BDD var = fe.getVariable(this.variable);
        return var.and(this.left.generateFE(fe)).or(var.not().and(this.right.generateFE(fe)));
    }
}
