---
title: Belief Systems and God-Game Design (Black & White)
created: 2026-04-14
updated: 2026-04-14
status: active
tags: [reference, god-games, belief-systems, creature-ai, black-and-white]
---

# Belief Systems and God-Game Design

Evidence base for the design of the player-as-deity mechanic — specifically the **belief / favour cultivation loop** inherited from Black & White (Lionhead, 2001) and Black & White 2 (Lionhead, 2005). Genesis's divine-favour mechanic is modelled directly on this system; this doc collects what is actually documented about how it was designed and why it worked.

Informs the design decision captured in [[../game-concepts/genesis-god-sim#divine-mythology-champions|Genesis → Divine Mythology & Champions]].

---

## Why this doc exists separately

Most of what is publicly documented about Black & White is **practitioner-tier** (Tier 5): Molyneux GDC talks, Evans's AI Game Programming Wisdom chapters, Gamasutra postmortems, interviews. There is **no peer-reviewed game-studies paper** that takes Black & White as a primary case study. This is a real gap, not an under-search — confirmed by multiple search passes — and it should be treated as a constraint on what design decisions can be justified by citation alone. When Genesis's divine-favour mechanic makes a claim like *"the player is pulled toward cradling one species"*, we can point at Molyneux interviews and Evans's AI chapter but not at empirical evidence that the mechanic works.

The separation from the [[historical-narrative-generation|historical narrative generation reference]] doc is deliberate: B&W's contribution to Genesis is the **belief cultivation loop** (a player-psychology mechanic), not the chronicle / story-sifting framework (which B&W does not have). Keeping the two reference bases distinct makes it easier to evaluate each on its own evidence.

---

## 1. Richard Evans's Creature AI work

Richard Evans was Lionhead's lead AI programmer on Black & White and is the single most important primary source on the Creature AI. He later worked on The Sims 3, then Versu at Linden Lab with Emily Short, then moved to academic AI research at Imperial College (DeepMind-adjacent). His Google Scholar profile ([link](https://scholar.google.com/citations?user=2tbESYUAAAAJ&hl=en)) lists the canonical publication record; his personal page at Imperial is [doc.ic.ac.uk/~re14](https://www.doc.ic.ac.uk/~re14/).

### Key sources

- (Tier 5: Evans, R., 2002, *"Varieties of Learning"*, chapter in *AI Game Programming Wisdom* vol. 1, ed. Steve Rabin, Charles River Media, pp. 567–578) — the canonical practitioner chapter on the Creature AI. Describes the hybrid architecture: **decision trees + reinforcement learning + a BDI-style (belief / desire / intention) representation** for the Creature's mental state. Evans is credited with four chapters across the AI Game Programming Wisdom series by the Lionhead team; only "Varieties of Learning" is verifiable in the first volume's confirmed TOC. Full chapter text is paywalled; the bibliographic details come from WorldCat ([record](https://search.worldcat.org/title/ai-game-programming-wisdom/oclc/48691373)) and the Wikipedia entry on Evans ([link](https://en.wikipedia.org/wiki/Richard_Evans_(AI_researcher))).

- (Tier 5: "Mind Games", *IEEE Spectrum*, undated, [link](https://spectrum.ieee.org/mind-games)) — IEEE Spectrum profile describing the Creature AI architecture as combining **BDI modelling, decision trees, neural nets, and empathic learning** (the Creature observes the player's actions and imitates them). This is the clearest single-paragraph summary of the architecture; Tier 5 because it is a magazine profile, not peer-reviewed.

- (Tier 2: Evans, R. & Short, E., 2014, *"Versu—A Simulationist Storytelling System"*, IEEE Transactions on Computational Intelligence and AI in Games, [Semantic Scholar](https://www.semanticscholar.org/paper/Versu%E2%80%94A-Simulationist-Storytelling-System-Evans-Short/74c6364ae004ce58e3f15a20c1e6d22198a93e21)) — Evans's and Short's multi-agent social-simulation architecture built at Linden Lab. Retrospectively contrasts Versu's approach with Creature AI. Tier 2 because IEEE TCIAIG is peer-reviewed. **This is the only Tier 2 source in the B&W literature chain**, and it is not *about* B&W — it is Evans's post-B&W work discussing what he carried forward.

- (Tier 5: Evans, R., GDC 2010, *"Modeling Individual Personalities in The Sims 3"*, [GDC Vault](https://www.gdcvault.com/play/1012450/Modeling-Individual-Personalities-in-The)) — post-Lionhead talk contrasting The Sims 3's personality modelling with the Creature AI's BDI approach. Useful for understanding what Evans considered the key lessons from B&W.

- (Tier 4: Wexler, J., 2002, *"Artificial Intelligence in Games: A look at the smarts behind Lionhead Studio's Black and White"*, Univ. of Rochester undergraduate CS term paper, [PDF](https://www.cs.rochester.edu/~brown/242/assts/termprojs/games.pdf)) — widely cited secondary description of the Creature AI. Describes the use of a **partial-order planner, game-state search, and a dynamic lookup table**. Not peer-reviewed; Tier 4 as a student paper. Useful only as a secondary summary because no better primary source is publicly available.

### What the architecture actually is (consensus from the above)

The Creature's decision-making is a hybrid of:

- **Belief/Desire/Intention (BDI) modelling** — the Creature maintains symbolic representations of what it *believes* to be true about the world, what it *desires*, and what it currently *intends* to do. This is classical agent AI from the 1990s.
- **Decision trees** — compiled from observed player actions. When the player rewards or punishes a Creature action, that feedback updates the tree.
- **Reinforcement learning** — Q-learning-adjacent feedback on outcomes; Evans has described this in both the book chapter and the Spectrum profile.
- **Empathic / observational learning** — the Creature watches what the player does and imitates it, even without explicit training.

The key design consequence for EcoSim: the Creature AI in B&W is **not a belief system for villages** — it is a *learning system for the pet creature*. The village belief mechanic is a separate system that Evans's chapters do not primarily cover. This is a common misconception; the Creature AI and the village belief system are orthogonal in B&W's architecture, and they only interact because a well-trained Creature can perform miracles that generate belief for villages.

---

## 2. Belief system design — direct developer sources

The belief / village-alignment system is less documented than the Creature AI. Molyneux discussed it in multiple interviews and the GDC 2000 introduction, but there is no single authoritative source equivalent to Evans's AI Game Programming Wisdom chapter.

### Key sources

- (Tier 5: Molyneux, P., 2001, *"Postmortem: Lionhead Studios' Black & White"*, Game Developer / Gamasutra, [link](https://www.gamedeveloper.com/design/postmortem-lionhead-studios-i-black-white-i-)) — the canonical postmortem. **Important caveat:** the postmortem focuses on development process, schedule, team structure, and the Creature AI. It does *not* directly discuss the belief system mechanics. This was verified by fetching the article body. The belief system is documented elsewhere, not here.

- (Tier 5: Molyneux, P., GDC 2000, *"Black & White" introduction keynote excerpt*, [GDC Vault](https://www.gdcvault.com/play/1014842/Excerpt-Peter-Molyneux-introduces-Black)) — pre-release keynote discussing the player-as-god intent. Excerpt only; full transcript not available.

- (Tier 5: *"Postcard from GDC Europe 2005: Black and White 2: How to Design a Giant"*, Game Developer, [link](https://www.gamedeveloper.com/design/postcard-from-gdc-europe-2005-i-black-and-white-2-i-how-to-design-a-giant)) — B&W 2 design talk reportage. Describes town-centre fountain and statue mechanics as **diegetic belief/impressiveness indicators** — the design team explicitly pushed toward in-world visual feedback for belief rather than menu-based numbers. This is the clearest public statement about how the belief system should *feel* to the player.

- (Tier 5: Molyneux interview, *"Peter Molyneux: The Essence of Interaction"*, Gamasutra, [link](https://www.gamedeveloper.com/design/peter-molyneux-the-essence-of-interaction)) — retrospective on interaction design; mentions sustaining belief through the god-player's actions but without mechanical specifics.

- (Tier 5: Grossman, A. ed., 2003, *Postmortems from Game Developer*, Routledge, [record](https://www.routledge.com/Postmortems-from-Game-Developer-Insights-from-the-Developers-of-Unreal-Tournament-Black-amp-White-Age-of-Empire-and-Other-Top-Selling-Games/Grossman/p/book/9781578202140)) — reprint anthology including the B&W postmortem. Same content as the Gamasutra version, but potentially with editorial framing not in the online post.

### What can be stated from these sources (with provenance)

- **Belief is generated by miracles and by Creature behaviour on behalf of the player.** Widely stated in developer interviews; not pinned to a specific mechanical formula.
- **Belief spreads geographically** — nearby villages can be converted by sustained miracle activity radiating from a converted village. Implied by playthrough evidence and confirmed in interview snippets; no direct formula published.
- **The B&W 2 redesign explicitly pushed belief indicators into the diegetic UI** — town-centre fountains, statues, and visible ceremonies were added as replacements for menu-based belief numbers. This is documented in the GDC Europe 2005 postcard.
- **Design intent: the player should be pulled toward one or a few favoured villages rather than spreading effort uniformly.** This is the transferable mechanic for Genesis but is not stated as cleanly in a single citation as we'd like — the closest is Molyneux's "Essence of Interaction" interview discussing how belief-sustenance creates attachment.

**Honest assessment:** the specific "cradle a favoured village" dynamic that Genesis's divine-favour system is modelled on is *folkloric* in the game design community — everyone who played B&W knows the game pulls you toward a favoured village, but the design documentation is thin. Any Genesis design decision that cites this mechanism is ultimately relying on community consensus about how the game *felt to play*, not on published design data.

---

## 3. Academic analyses of Black & White

**No peer-reviewed game-studies paper uses Black & White as a primary case study.** Multiple search passes confirmed this gap. The one paper that surfaces on "god games" academic searches — Hajikhameneh & Iannaccone, 2023, *"God games: An experimental study of uncertainty, superstition, and cooperation"*, *Games and Economic Behavior* — is an experimental economics study using the phrase metaphorically; it is not about the Lionhead title.

Black & White is referenced in passing in broader discussions of god games, moral alignment mechanics, and emergent narrative, but it is not the subject of a dedicated peer-reviewed study that we could find. This is worth noting because it means any design claims made on B&W's authority are **not academically citable** — they rest on practitioner sources only.

---

## 4. Adjacent theory

Frameworks that inform the belief-cultivation loop without directly analysing B&W:

- (Tier 5: Schell, J., 2008, *The Art of Game Design: A Book of Lenses*, Elsevier/CRC, [Taylor & Francis](https://www.taylorfrancis.com/books/mono/10.1201/b17723/art-game-design-jesse-schell)) — standard practitioner text on game design lenses. Applicable to god-game progression loops; no confirmed B&W-specific section found in search snippets.

- (Tier 5: Sicart, M., 2009, *The Ethics of Computer Games*, MIT Press, [MIT Press](https://mitpress.mit.edu/9780262516624/the-ethics-of-computer-games/); and *"Wicked Games"*, [PDF](https://miguelsicart.net/publications/Wicked%20Games.pdf)) — framework on moral subjecthood in games. Sicart critiques *Fable* (Molyneux's subsequent game) for imposing morality rather than letting players construct it. Directly relevant to Genesis's divine-favour mechanic: if the player's alignment is scored, the game is making a moral judgement on their behalf, which Sicart argues is usually the wrong move. A design implication worth considering: favour should not be "good vs evil" but "how this species regards you", which is a more specific claim.

- (Tier 5: Lazzaro, N., 2004, *"Why We Play Games: Four Keys to More Emotion Without Story"*, [ResearchGate](https://www.researchgate.net/publication/248446107_Why_we_Play_Games_Four_Keys_to_More_Emotion_without_Story)) — the "People Fun" and "Hard Fun / fiero" keys map onto the cradle-a-species loop: investment produces pride, pride produces attachment, attachment produces continued engagement. Useful framing for *why* the loop works without naming B&W.

- (Tier 5: Bartle, R., 1996, *"Hearts, Clubs, Diamonds, Spades: Players Who Suit MUDs"*, [Wikipedia summary](https://en.wikipedia.org/wiki/Bartle_taxonomy_of_player_types)) — player-type taxonomy. Relevant for thinking about which players (Achievers vs Explorers vs Socialisers vs Killers) are drawn to nurturing a single species. The cradle loop is most appealing to Socialisers and Achievers; Explorers will ignore it; Killers will invert it (cultivate a species to watch it fall). Genesis should accommodate all four if possible.

---

## 5. Implications for Genesis (not decisions)

This section deliberately does not make design decisions — those live in [[../game-concepts/genesis-god-sim|the Genesis doc]]. It lists what the evidence base can and cannot support.

**What the evidence base supports:**

- Divine favour as a species-level scalar fed by player interventions — supported by B&W's village belief system via practitioner consensus; formal mechanics undocumented.
- Tiered outcomes feeding the favour delta — supported by Evans's Creature AI architecture (reinforcement learning on player feedback) translated from creature-scoped to species-scoped.
- Diegetic indicators rather than menu numbers — directly supported by the B&W 2 design postcard (town-centre fountains, statues, visible ceremonies).
- Geographic / ecological spread of favour (nearby species influenced by a worshipping species) — supported by B&W village conversion mechanics via playthrough evidence; formal mechanics undocumented.

**What the evidence base does NOT support (but Genesis may still want to do):**

- The specific claim that "the player is pulled toward cradling one species" is *folkloric* — it is real community consensus but has no citable source. Any Genesis design that relies on this pull working should be validated by playtesting, not by citation.
- "Good vs evil" alignment — Sicart's critique applies here. Genesis's favour should be per-species (how this species regards you), not global (your moral alignment).
- Explicit thresholds and formulas for favour gain/loss rates — no public source has formulas to copy. All numbers must be tuned empirically.

---

## 6. Gaps in this reference base

Areas where this doc cannot make strong claims because the primary material is missing:

- **Full text of Evans's "Varieties of Learning"** — paywalled. The description above is assembled from secondary summaries (IEEE Spectrum, Wikipedia, Wexler 2002). A reader with the book should be able to verify the BDI + decision-tree + RL architecture claims.
- **Full Molyneux postmortem on design of the belief system specifically** — the postmortem does not cover it. A deeper design document or a specific GDC talk segment would be needed, and neither has been located.
- **Any peer-reviewed study of Black & White.** Confirmed gap across multiple searches.
- **Formal documentation of the favour-accrual / favour-spread formulas.** Not published.
- **Evans's AIIDE 2007 invited talk.** Confirmed he spoke but the abstract and slides are not recoverable.

**Rerun recommendation.** A second research pass with full GDC Vault video access, university library access to *AI Game Programming Wisdom* vol. 1, and direct MIT Press access to Sicart's book would significantly strengthen the primary-source coverage. The current doc is sufficient for Genesis v1 design decisions but should be updated when any of those three become available.

---

## References (consolidated)

### Practitioner sources (Tier 5)

- Evans, R. (2002). *Varieties of Learning*. In *AI Game Programming Wisdom* vol. 1 (ed. S. Rabin). Charles River Media.
- Evans, R. Game Developer / Gamasutra author page: [link](https://www.gamedeveloper.com/author/richard-evans)
- Evans, R. (GDC 2010). *Modeling Individual Personalities in The Sims 3*: [GDC Vault](https://www.gdcvault.com/play/1012450/Modeling-Individual-Personalities-in-The)
- Molyneux, P. (2001). *Postmortem: Lionhead Studios' Black & White*: [Game Developer](https://www.gamedeveloper.com/design/postmortem-lionhead-studios-i-black-white-i-)
- Molyneux, P. (GDC 2000). *Black & White introduction keynote excerpt*: [GDC Vault](https://www.gdcvault.com/play/1014842/Excerpt-Peter-Molyneux-introduces-Black)
- *Postcard from GDC Europe 2005: Black and White 2: How to Design a Giant*: [Game Developer](https://www.gamedeveloper.com/design/postcard-from-gdc-europe-2005-i-black-and-white-2-i-how-to-design-a-giant)
- *Peter Molyneux: The Essence of Interaction*: [Game Developer](https://www.gamedeveloper.com/design/peter-molyneux-the-essence-of-interaction)
- *Mind Games*, IEEE Spectrum: [link](https://spectrum.ieee.org/mind-games)
- Grossman, A. ed. (2003). *Postmortems from Game Developer*. Routledge: [record](https://www.routledge.com/Postmortems-from-Game-Developer-Insights-from-the-Developers-of-Unreal-Tournament-Black-amp-White-Age-of-Empire-and-Other-Top-Selling-Games/Grossman/p/book/9781578202140)

### Academic / secondary sources

- (Tier 4) Wexler, J. (2002). *Artificial Intelligence in Games: A look at the smarts behind Lionhead Studio's Black and White*. Univ. of Rochester CS term paper: [PDF](https://www.cs.rochester.edu/~brown/242/assts/termprojs/games.pdf)
- (Tier 2) Evans, R. & Short, E. (2014). *Versu—A Simulationist Storytelling System*. IEEE TCIAIG: [Semantic Scholar](https://www.semanticscholar.org/paper/Versu%E2%80%94A-Simulationist-Storytelling-System-Evans-Short/74c6364ae004ce58e3f15a20c1e6d22198a93e21)
- (Tier 5) Schell, J. (2008). *The Art of Game Design*. Elsevier/CRC.
- (Tier 5) Sicart, M. (2009). *The Ethics of Computer Games*. MIT Press.
- (Tier 5) Lazzaro, N. (2004). *Why We Play Games: Four Keys to More Emotion Without Story*: [ResearchGate](https://www.researchgate.net/publication/248446107)
- (Tier 5) Bartle, R. (1996). *Hearts, Clubs, Diamonds, Spades: Players Who Suit MUDs*: [summary](https://en.wikipedia.org/wiki/Bartle_taxonomy_of_player_types)

### Evans publication roots (starting points for further research)

- Personal page: [doc.ic.ac.uk/~re14](https://www.doc.ic.ac.uk/~re14/)
- Google Scholar: [profile](https://scholar.google.com/citations?user=2tbESYUAAAAJ&hl=en)
- Wikipedia: [Richard Evans (AI researcher)](https://en.wikipedia.org/wiki/Richard_Evans_(AI_researcher))

---

## See also

- [[../game-concepts/genesis-god-sim#divine-mythology-champions]] — the Genesis divine-favour mechanic that consumes this research
- [[historical-narrative-generation]] — parallel reference doc on chronicle / story-sifting systems (distinct topic)
- [[../game-concepts/README#influences]] — Black & White as one of EcoSim's canonical influences
