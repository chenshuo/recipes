package billing;

import groovy.lang.GroovyClassLoader;

import java.io.File;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

public class RuleFactory {

    private static class CachedRule {
        public final long lastModified;
        public final Rule rule;

        public CachedRule(long lastModified, Rule rule) {
            this.lastModified = lastModified;
            this.rule = rule;
        }
    }

    //private final CachedRule kDefaultRule = new CachedRule(0, new DefaultRule());

    private final Map<String, CachedRule> rules = new ConcurrentHashMap<String, CachedRule>();
    private final String rulePath;

    public RuleFactory(String rulePath) {
        this.rulePath = rulePath;
    }

    Rule create(String name) {
        return create(name, null);
    }

    Rule create(String name, Object state) {
        // System.out.println("creating rule " + name);
        CachedRule entry = rules.get(name);
        File file = getGroovyFile(name);
        if (file.exists()) {
            if (entry == null || entry.lastModified < file.lastModified()) {
                entry = loadGroovy(file);
                rules.put(name, entry);
            }
        }

        if (entry == null) {
            entry = new CachedRule(0, new DefaultRule(name));
            System.err.println("Failed to create rule '" + name + "'");
        }

        Rule result = entry.rule;

        if (state != null) {
          result = (Rule)entry.rule.clone();
          result.setState(state);
        }

        return result;
    }

    private CachedRule loadGroovy(File file) {
        CachedRule entry = null;
        try {
            GroovyClassLoader gcl = new GroovyClassLoader();
            Class<?> clazz = gcl.parseClass(file);
            Rule rule = (Rule) clazz.newInstance();
            rule.setFactory(this);
            entry = new CachedRule(file.lastModified(), rule);
        } catch (Exception e) {
            e.printStackTrace();
        }
        return entry;
    }

    private File getGroovyFile(String name) {
        return new File(rulePath + name + ".groovy");
    }
}
