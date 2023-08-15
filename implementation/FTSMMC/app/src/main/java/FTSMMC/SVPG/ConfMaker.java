package FTSMMC.SVPG;

import net.sf.javabdd.BDD;

public class ConfMaker {
    private SetMaker setmaker;
    private SetSizer setsizer;

    public static ConfMaker getConfMaker(String type, float lambda) throws Exception {
        switch (type) {
            case "FF":
                return new ConfMaker(
                        new BernoulliSetSizer(lambda),
                        new FeaturedSetMaker());
            case "FC":
                return new ConfMaker(
                        new BernoulliSetSizer(lambda),
                        new ConfigurationSetMaker());
            case "BF":
                return new ConfMaker(
                        new BetaSetSizer(lambda),
                        new FeaturedSetMaker());
            case "BC":
                return new ConfMaker(
                        new BetaSetSizer(lambda),
                        new ConfigurationSetMaker());
            default:
                throw new Exception("Unknown random configuration: " + type);
        }
    }

    private ConfMaker(SetSizer setsizer, SetMaker setmaker) {
        this.setmaker = setmaker;
        this.setsizer = setsizer;
    }

    public BDD getSet() {
        return setmaker.makeSet(setsizer.getSetSize());
    }
}
