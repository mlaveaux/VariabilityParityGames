package mmc.models;

import mmc.features.BDDParser;
import mmc.features.FeatureDiagram;
import net.sf.javabdd.BDD;

import java.util.Objects;

public class Label {
    private final String text;

    private BDD featureExpression;

    public Label(String text){
        this(text, true);
    }
    public Label(String text, boolean parseFE) {
        int s = text.indexOf('(');
        if(!parseFE || s == -1)
        {
            this.text = text;
            this.featureExpression = FeatureDiagram.PrimaryFD.factory.one();
        } else {
            int e = text.lastIndexOf(')');
            this.text = text.substring(0, s);
            try {
                this.featureExpression = BDDParser.parseBDD(text.substring(s+1,e), FeatureDiagram.PrimaryFD);
            } catch (Exception e1) {
                e1.printStackTrace();
            }
        }
    }

    public String getText() {
        return this.text;
    }

    public BDD getFeatureExpression(){
        return this.featureExpression;
    }
    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;
        Label label1 = (Label) o;
        return Objects.equals(this.text, label1.text) && !this.featureExpression.and(label1.featureExpression).isZero();
    }

    @Override
    public int hashCode() {

        return Objects.hash(this.text, featureExpression);
    }

    @Override
    public String toString() {
        return this.text;
    }

    public boolean satisfies(BDD product)
    {
        return !product.and(featureExpression).isZero();
    }
}
