package billing;

public abstract class Rule implements Cloneable {

    protected RuleFactory factory;
    protected Object state;

    public void setFactory(RuleFactory factory) {
        this.factory = factory;
    }

    public void setState(Object state) {
        this.state = state;
    }

    @Override
    protected Object clone() {
        Class<? extends Rule> clazz = getClass();
        Rule newRule = null;
        try {
            newRule = clazz.newInstance();
            newRule.setFactory(factory);
        } catch (Exception e) {
            e.printStackTrace();
        }
        return newRule;
    }

    public abstract long getMoneyInPips(UserMonthUsage input);
}
