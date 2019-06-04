package mmc.models;

import mmc.features.BDDParser;
import mmc.features.FeatureDiagram;

import java.util.Objects;

public class Label {
    private final String text;

    private int featureExpression;

    public Label(String text) {
        int s = text.indexOf('(');
        if(s == -1)
        {
            this.text = text;
            this.featureExpression = 1;
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

    public int getFeatureExpression(){
        return this.featureExpression;
    }
    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;
        Label label1 = (Label) o;
        return Objects.equals(this.text, label1.text) && FeatureDiagram.PrimaryFD.and(this.featureExpression, label1.featureExpression) > 0;
    }

    @Override
    public int hashCode() {

        return Objects.hash(this.text, featureExpression);
    }

    @Override
    public String toString() {
        return this.text;
    }

    public boolean satisfies(int product)
    {
        return FeatureDiagram.PrimaryFD.and(product, featureExpression) > 0;
    }
}
