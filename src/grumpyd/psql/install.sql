-- This file installs the default database backend for grumpy

/******** users ********/

CREATE TABLE public.users (
    id integer NOT NULL,
    name text NOT NULL,
    password text NOT NULL,
    last_login numeric,
    role text NOT NULL,
    is_locked boolean DEFAULT false NOT NULL
);

COMMENT ON TABLE public.users IS 'Users';

CREATE SEQUENCE public.users_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;

ALTER SEQUENCE public.users_id_seq OWNED BY public.users.id;

ALTER TABLE ONLY public.users ALTER COLUMN id SET DEFAULT nextval('public.users_id_seq'::regclass);

ALTER TABLE ONLY public.users
    ADD CONSTRAINT users_name_key UNIQUE (name);

ALTER TABLE ONLY public.users
    ADD CONSTRAINT users_pkey PRIMARY KEY (id);

/******** USER_DATA ********/

CREATE TABLE public.user_data (
    id integer NOT NULL,
    user_id numeric NOT NULL,
    key text NOT NULL,
    data bytea
);

COMMENT ON TABLE public.users IS 'Personal storage for users';

CREATE SEQUENCE public.user_data_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;

ALTER SEQUENCE public.user_data_id_seq OWNED BY public.user_data.id;

ALTER TABLE ONLY public.user_data ALTER COLUMN id SET DEFAULT nextval('public.user_data_id_seq'::regclass);

ALTER TABLE ONLY public.user_data
    ADD CONSTRAINT user_data_pkey PRIMARY KEY (id);


/******** roles ********/

CREATE TABLE public.roles (
    id integer NOT NULL,
    name text NOT NULL
);

CREATE SEQUENCE public.roles_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;

ALTER SEQUENCE public.roles_id_seq OWNED BY public.roles.id;

ALTER TABLE ONLY public.roles ALTER COLUMN id SET DEFAULT nextval('public.roles_id_seq'::regclass);

ALTER TABLE ONLY public.roles
    ADD CONSTRAINT roles_name_key UNIQUE (name);

ALTER TABLE ONLY public.roles
    ADD CONSTRAINT roles_pkey PRIMARY KEY (id);

/******** privileges ********/

CREATE TABLE public.privileges (
    id integer NOT NULL,
    name text
);

CREATE SEQUENCE public.privileges_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER SEQUENCE public.privileges_id_seq OWNED BY public.privileges.id;

ALTER TABLE ONLY public.privileges ALTER COLUMN id SET DEFAULT nextval('public.privileges_id_seq'::regclass);

ALTER TABLE ONLY public.privileges
    ADD CONSTRAINT privileges_name_key UNIQUE (name);

ALTER TABLE ONLY public.privileges
    ADD CONSTRAINT privileges_pkey PRIMARY KEY (id);

/******** scrollbacks ********/

CREATE TABLE public.scrollbacks (
    id integer NOT NULL,
    original_id numeric NOT NULL,
    user_id numeric NOT NULL,
    parent numeric,
    target text NOT NULL,
    last_item numeric NOT NULL,
    type numeric NOT NULL,
    virtual_state numeric NOT NULL,
    item_count numeric,
    is_hidden boolean DEFAULT false NOT NULL,
    property bytea
);

CREATE SEQUENCE public.scrollbacks_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;

ALTER SEQUENCE public.scrollbacks_id_seq OWNED BY public.scrollbacks.id;

ALTER TABLE ONLY public.scrollbacks ALTER COLUMN id SET DEFAULT nextval('public.scrollbacks_id_seq'::regclass);

ALTER TABLE ONLY public.scrollbacks
    ADD CONSTRAINT scrollbacks_pkey PRIMARY KEY (id);

CREATE INDEX scrollbacks_user_idx ON scrollbacks USING btree (user_id)

/******** networks ********/

CREATE TABLE public.networks (
    id integer NOT NULL,
    network_id numeric NOT NULL,
    user_id numeric NOT NULL,
    name text,
    hostname text,
    port numeric NOT NULL,
    ssl boolean NOT NULL,
    nick text NOT NULL,
    ident text NOT NULL,
    system_id numeric NOT NULL,
    password text,
    scrollback_list text,
    autoreconnect boolean DEFAULT false NOT NULL,
    autoidentify boolean DEFAULT false NOT NULL,
    autorejoin boolean DEFAULT false NOT NULL
);

CREATE SEQUENCE public.networks_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;

ALTER SEQUENCE public.networks_id_seq OWNED BY public.networks.id;

ALTER TABLE ONLY public.networks ALTER COLUMN id SET DEFAULT nextval('public.networks_id_seq'::regclass);

ALTER TABLE ONLY public.networks
    ADD CONSTRAINT networks_pkey PRIMARY KEY (id);

/******** scrollback_items ********/

CREATE TABLE public.scrollback_items (
    id integer NOT NULL,
    item_id numeric NOT NULL,
    user_id numeric NOT NULL,
    scrollback_id numeric NOT NULL,
    date numeric NOT NULL,
    type numeric NOT NULL,
    nick text,
    ident text,
    host text,
    text text,
    self boolean DEFAULT false NOT NULL
);

CREATE SEQUENCE public.scrollback_items_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;

ALTER SEQUENCE public.scrollback_items_id_seq OWNED BY public.scrollback_items.id;

ALTER TABLE ONLY public.scrollback_items ALTER COLUMN id SET DEFAULT nextval('public.scrollback_items_id_seq'::regclass);

ALTER TABLE ONLY public.scrollback_items
    ADD CONSTRAINT scrollback_items_pkey PRIMARY KEY (id);

CREATE INDEX scrollback_items_user_idx ON public.scrollback_items USING btree (user_id);
CREATE INDEX scrollback_items_user_scrollback_idx ON public.scrollback_items USING btree (user_id, scrollback_id);
CREATE INDEX scrollback_items_user_scrollback_item_idx ON public.scrollback_items USING btree (item_id, user_id, scrollback_id);

/******** settings ********/

CREATE TABLE public.settings (
    id integer NOT NULL,
    user_id numeric NOT NULL,
    value bytea
);

CREATE SEQUENCE public.settings_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;

ALTER SEQUENCE public.settings_id_seq OWNED BY public.settings.id;

ALTER TABLE ONLY public.settings ALTER COLUMN id SET DEFAULT nextval('public.settings_id_seq'::regclass);

ALTER TABLE ONLY public.settings
    ADD CONSTRAINT settings_pkey PRIMARY KEY (id);

/******** meta ********/

CREATE TABLE public.meta (
    key text NOT NULL,
    value text
);

ALTER TABLE ONLY public.meta
    ADD CONSTRAINT meta_key UNIQUE (key);

/* This is just to track the version information for updates */
INSERT INTO meta (key, value) VALUES ('version', '1');

